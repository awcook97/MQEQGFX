/*
 * Models.cpp - experimental model viewer (WIP). Parses the subject's model
 * from the EQ folder (model/ readers), repaints it to the captured worn look,
 * and renders it in an ImGui window. Also owns the appearance capture.
 */

#include "MQEQGFX.h"

#include "model/pfs.h"
#include "model/wld.h"

#include <imgui.h>

#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// Live-captured appearance (shared decls in MQEQGFX.h). The engine builds a
// character's worn look by renaming body materials (ReplaceMaterial) and
// picking a worn head (SwapHead); we record that stream from the local actor
// and replay it at render time. Persisted per-ActorDef.
std::mutex             s_capMutex;
std::vector<MatEvent>  s_matEvents;
std::string            s_headMesh;
uint32_t               s_headTint = 0;
bool                   s_haveCapture = false;
std::string            s_captureActorDef;
std::atomic<bool>      s_captureDirty{ false };
bool                   s_captureInstalled = false;
bool                   s_captureVerbose = false;

// ---------------------------------------------------------------------------
// Self-parsed model view (our own DX11 render of a parsed EQG model into an
// ImGui window). The parsed model is handed from the command thread to the
// render thread (OnUpdateImGui) under s_modelMutex.
eqgfx::render::ModelView s_modelView;                 // render thread only
std::atomic<bool>        s_showModel{ false };
std::atomic<bool>        s_reloadModel{ false };      // ImGui asks main thread to rebuild

namespace {

std::mutex                     s_modelMutex;
eqgfx::eqg::Model              s_pendingModel;        // guarded
std::vector<eqgfx::dds::Image> s_pendingTextures;     // guarded (parallel to materials)
bool                           s_modelDirty = false;  // guarded

// Worn-head placement tweak (degrees / milli-world-Z)
std::atomic<int>         s_headYawDeg{ 0 };
std::atomic<int>         s_headPitchDeg{ 0 };
std::atomic<int>         s_headRollDeg{ 180 };
std::atomic<int>         s_headDzMilli{ 500 };

// Lowercase and strip a trailing "_mdf" (WLD material suffix).
std::string NormMat(std::string s)
{
	for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (s.size() > 4 && s.compare(s.size() - 4, 4, "_mdf") == 0) s.erase(s.size() - 4);
	return s;
}

// Substring search over a raw buffer (classifies armor pieces by bone names).
bool BufContains(const std::vector<uint8_t>& b, const char* needle)
{
	size_t n = std::strlen(needle);
	if (b.size() < n) return false;
	for (size_t i = 0; i + n <= b.size(); ++i)
		if (std::memcmp(b.data() + i, needle, n) == 0) return true;
	return false;
}

// Append src (+ its textures) into dst, rebasing indices/materials.
void AppendModel(eqgfx::eqg::Model& dst, std::vector<eqgfx::dds::Image>& dstTex,
	const eqgfx::eqg::Model& src, const std::vector<eqgfx::dds::Image>& srcTex)
{
	uint32_t vbase = static_cast<uint32_t>(dst.vertices.size());
	uint32_t ibase = static_cast<uint32_t>(dst.indices.size());
	int mbase = static_cast<int>(dst.materials.size());
	dst.vertices.insert(dst.vertices.end(), src.vertices.begin(), src.vertices.end());
	for (uint32_t idx : src.indices) dst.indices.push_back(vbase + idx);
	for (const eqgfx::eqg::SubMesh& sm : src.submeshes)
	{
		eqgfx::eqg::SubMesh s = sm;
		s.firstIndex += ibase;
		if (s.materialIndex >= 0) s.materialIndex += mbase;
		dst.submeshes.push_back(std::move(s));
	}
	dst.materials.insert(dst.materials.end(), src.materials.begin(), src.materials.end());
	dstTex.insert(dstTex.end(), srcTex.begin(), srcTex.end());
	for (int i = 0; i < 3; ++i)
	{
		if (src.minB[i] < dst.minB[i]) dst.minB[i] = src.minB[i];
		if (src.maxB[i] > dst.maxB[i]) dst.maxB[i] = src.maxB[i];
	}
}

int LoadArmorOverlay(const std::string& dir, const std::string& code, const std::string& type,
	eqgfx::eqg::Model& model, std::vector<eqgfx::dds::Image>& textures)
{
	eqgfx::pfs::Archive a;
	if (!a.Open(dir + "velious_" + type + "_" + code + ".eqg")) return 0;

	auto ends = [](const std::string& s, const char* e) {
		size_t n = std::strlen(e);
		return s.size() >= n && s.compare(s.size() - n, n, e) == 0;
	};
	// One representative piece per region (first legs, first left/right foot).
	std::string legs, lfoot, rfoot;
	for (const std::string& m : a.MemberNames())
	{
		if (!ends(m, ".mod")) continue;
		std::vector<uint8_t> buf;
		if (!a.Extract(m, buf)) continue;
		bool bl = BufContains(buf, "BOFootL"), br = BufContains(buf, "BOFootR");
		if (bl) { if (lfoot.empty()) lfoot = m; }
		else if (br) { if (rfoot.empty()) rfoot = m; }
		else if (legs.empty() && BufContains(buf, "CACalfL") && BufContains(buf, "CACalfR")) legs = m;
	}

	int merged = 0;
	for (const std::string& m : { legs, lfoot, rfoot })
	{
		if (m.empty()) continue;
		std::vector<uint8_t> buf;
		if (!a.Extract(m, buf)) continue;
		eqgfx::eqg::Model piece;
		std::string err;
		if (!eqgfx::eqg::ParseModel(buf.data(), buf.size(), piece, err)) continue;
		std::vector<eqgfx::dds::Image> ptex(piece.materials.size());
		for (size_t i = 0; i < piece.materials.size(); ++i)
		{
			const std::string& t = piece.materials[i].diffuseTexture;
			if (t.empty() || t == "None") continue;
			std::vector<uint8_t> tb;
			if (a.Extract(t, tb)) eqgfx::dds::ParseTexture(tb.data(), tb.size(), ptex[i]);
		}
		AppendModel(model, textures, piece, ptex);
		++merged;
	}
	return merged;
}

// Body texture region code ("HAFCH0001" -> "ch") to the ActorEquipment slot
// (0=Head..6=Feet). -1 = not swappable. "fa" is the forearm, not the face.
int RegionToSlot(const std::string& rr)
{
	if (rr == "ch") return 1;   // Chest
	if (rr == "ua") return 2;   // Arms (upper arm)
	if (rr == "fa") return 2;   // Arms (forearm)
	if (rr == "hn") return 4;   // Hands
	if (rr == "lg") return 5;   // Legs
	if (rr == "ft") return 6;   // Feet
	return -1;
}

// Old models: the game replaces haf<region>00<pp> with haf<region><MM><pp>
// where MM is the armor-type value, so the material number IS the armor type.
int ArmorTypeToMaterial(uint32_t armorType)
{
	return (armorType <= 9) ? static_cast<int>(armorType) : -1;
}

// Fallback armor look from equipment types: swap each body region's texture to
// the equipped item's armor material and apply its dye. WLD-only.
void ApplyEquipmentArmor(PlayerClient* subj, const std::string& code, const std::string& dir,
	eqgfx::pfs::Archive& baseArc, eqgfx::eqg::Model& model, std::vector<eqgfx::dds::Image>& textures,
	std::string& report)
{
	if (!subj) return;
	const auto& equip = subj->mActorClient.ActorEquipment;
	const uint32_t* armorColor = subj->mActorClient.ArmorColor;

	// Per-slot summary (once each), for diagnostics.
	static const char* kSlotName[7] = { "head", "chest", "arms", "wrist", "hands", "legs", "feet" };
	for (int s = 0; s < 7; ++s)
	{
		char line[128];
		std::snprintf(line, sizeof(line), "%s{ty=%u nid=%u nty=%u mat->%d tint=%06X} ", kSlotName[s],
			equip.Item[s].type, equip.Item[s].newArmorID, equip.Item[s].newArmorType,
			ArmorTypeToMaterial(equip.Item[s].type), armorColor[s] & 0xFFFFFF);
		report += line;
	}

	// The authentic classic armor textures (haf<region><MM><pp>) live in
	// global_chr.s3d (8-bit BMP; has every region for materials 00-03). Open once.
	eqgfx::pfs::Archive globalChr;
	bool haveGlobalChr = globalChr.Open(dir + "global_chr.s3d");

	auto tryLoad = [&](eqgfx::pfs::Archive* arc, int m, const std::string& rr, const std::string& pp,
		const char* ext, eqgfx::dds::Image& out) -> bool {
		if (!arc) return false;
		char tn[40];
		std::snprintf(tn, sizeof(tn), "%s%s%02d%s%s", code.c_str(), rr.c_str(), m, pp.c_str(), ext);
		std::vector<uint8_t> tb;
		out = eqgfx::dds::Image{};
		return arc->Extract(tn, tb) && eqgfx::dds::ParseTexture(tb.data(), tb.size(), out) && out.valid();
	};

	auto lower = [](std::string s) { for (char& c : s) c = static_cast<char>(std::tolower((unsigned char)c)); return s; };
	for (size_t i = 0; i < model.materials.size(); ++i)
	{
		std::string mn = lower(model.materials[i].name);   // e.g. "hafch0001_mdf"
		if (mn.size() < code.size() + 6 || mn.compare(0, code.size(), code) != 0) continue;
		std::string rr = mn.substr(code.size(), 2);
		std::string digits = mn.substr(code.size() + 2, 4);
		bool allDigit = true;
		for (char c : digits) if (!std::isdigit((unsigned char)c)) allDigit = false;
		if (!allDigit) continue;
		int slot = RegionToSlot(rr);
		if (slot < 0) continue;

		int mat = ArmorTypeToMaterial(equip.Item[slot].type);
		if (mat <= 0) continue;   // material 0 = base skin, no swap
		std::string pp = digits.substr(2, 2);

		// Exactly what the game does: replace haf<region>00<pp> with haf<region><mat><pp>
		// if that texture exists (global_chr .bmp, else body archive .dds); otherwise
		// leave the default (some pieces have no armor texture and stay skin, e.g. the
		// female chest piece the game never replaces).
		eqgfx::dds::Image img;
		if (haveGlobalChr) { tryLoad(&globalChr, mat, rr, pp, ".bmp", img); }
		if (!img.valid())  { tryLoad(&baseArc, mat, rr, pp, ".dds", img); }
		if (!img.valid())  { tryLoad(&globalChr, mat, rr, pp, ".dds", img); }

		if (img.valid())
		{
			textures[i] = std::move(img);
			uint32_t t = armorColor[slot] & 0xFFFFFF;
			model.materials[i].tint = t ? t : 0xFFFFFF;
		}
	}
}

// Repaint the model to the captured worn look: replay the rename stream per
// material (last write wins, like the engine's palette search), then load the
// final texture and tint. Returns false when there's no capture yet; the
// caller falls back to the equipment-type heuristic.
bool ApplyCapturedAppearance(const std::string& /*code*/, eqgfx::pfs::Archive& bodyArc,
	eqgfx::pfs::Archive* globalChr, eqgfx::eqg::Model& model,
	std::vector<eqgfx::dds::Image>& textures)
{
	std::vector<MatEvent> events; uint32_t headTint = 0; bool have = false;
	{
		std::scoped_lock lk(s_capMutex);
		have = s_haveCapture; events = s_matEvents; headTint = s_headTint;
	}
	if (!have) return false;

	auto tryTex = [&](eqgfx::pfs::Archive* arc, const std::string& fn, eqgfx::dds::Image& out) -> bool {
		if (!arc) return false;
		std::vector<uint8_t> tb; out = eqgfx::dds::Image{};
		return arc->Extract(fn, tb) && eqgfx::dds::ParseTexture(tb.data(), tb.size(), out) && out.valid();
	};

	for (size_t i = 0; i < model.materials.size(); ++i)
	{
		std::string M = NormMat(model.materials[i].name);
		if (M.empty()) continue;
		std::string cur = M; uint32_t tint = 0;
		for (const MatEvent& e : events) if (e.a == cur) { cur = e.b; tint = e.tint; }
		bool isHelm = cur.find("helm") != std::string::npos;
		if (isHelm && headTint) tint = headTint;

		// Only reload when the name actually changed - the default texture already
		// loaded is correct for untouched materials.
		if (cur != M)
		{
			eqgfx::dds::Image img;
			if (tryTex(globalChr, cur + ".bmp", img)
				|| tryTex(&bodyArc, cur + ".dds", img)
				|| tryTex(globalChr, cur + ".dds", img)
				|| tryTex(&bodyArc, cur + ".bmp", img))
				textures[i] = std::move(img);
		}
		if (tint) model.materials[i].tint = tint;
	}
	return true;
}

// Find + parse the subject's (target, else self) model from the EQ folder.
// Returns true with the parsed model + a source label; false with a 'note'.
bool LoadSubjectModel(eqgfx::eqg::Model& model, std::vector<eqgfx::dds::Image>& textures,
	std::string& src, std::string& note, const std::string& armorType = "",
	std::string* equipReport = nullptr)
{
	textures.clear();
	PlayerClient* subj = pTarget.get();
	if (!subj) subj = pLocalPlayer.get();
	if (!subj) { note = "no target or self"; return false; }

	std::string code = subj->mActorClient.ActorDef;
	size_t us = code.find('_');
	if (us != std::string::npos) code = code.substr(0, us);
	for (char& c : code) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	if (code.empty()) { note = "subject has no ActorDef tag"; return false; }

	char exePath[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, exePath, MAX_PATH);
	std::string dir(exePath);
	size_t slash = dir.find_last_of("\\/");
	dir = (slash != std::string::npos) ? dir.substr(0, slash + 1) : std::string();

	auto ends = [](const std::string& s, const char* e) {
		size_t n = std::strlen(e);
		return s.size() >= n && s.compare(s.size() - n, n, e) == 0;
	};

	std::string err;
	eqgfx::wld::BoneXform headBone;
	bool isWld = false;
	bool parsed = false;
	std::string bestArc, bestMember;

	// The original classic bodies + textures live in the shared global_chr.s3d;
	// try that first. Fall back to the per-race archive scan for races not in it
	// (e.g. the newer EQG races).
	eqgfx::pfs::Archive globalChr;
	bool haveGlobalChr = globalChr.Open(dir + "global_chr.s3d");
	if (haveGlobalChr)
	{
		std::vector<uint8_t> gbuf;
		if (globalChr.Extract("global_chr.wld", gbuf)
			&& eqgfx::wld::ParseCharacter(gbuf.data(), gbuf.size(), code, model, err, &headBone))
		{
			parsed = true; isWld = true;
			bestArc = "global_chr.s3d"; bestMember = code + "_DMSPRITEDEF (original)";
		}
	}

	// Fallback: scan the per-race archives for a body. Lower rank = better.
	eqgfx::pfs::Archive scanArc;
	if (!parsed)
	{
		std::vector<std::string> archives;
		for (const char* pat : { "*.eqg", "*.s3d" })
		{
			WIN32_FIND_DATAA fd;
			HANDLE h = FindFirstFileA((dir + pat).c_str(), &fd);
			if (h == INVALID_HANDLE_VALUE) continue;
			do {
				std::string nm = fd.cFileName, lnm = nm;
				for (char& c : lnm) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				if (lnm.find(code) != std::string::npos) archives.push_back(nm);
			} while (FindNextFileA(h, &fd));
			FindClose(h);
		}
		int bestRank = 99;
		for (const std::string& name : archives)
		{
			eqgfx::pfs::Archive a;
			if (!a.Open(dir + name)) continue;
			for (const std::string& m : a.MemberNames())
			{
				bool mds = ends(m, ".mds"), mod = ends(m, ".mod"), wld = ends(m, ".wld");
				if (!mds && !mod && !wld) continue;
				bool hasCode = m.find(code) != std::string::npos;
				int rank = (m == code + ".mds" || m == code + ".mod") ? 0
					: (m == "global" + code + "_chr.wld") ? 1
					: (wld && hasCode) ? 2
					: ((mds || mod) && hasCode) ? 8 : 9;
				if (rank < bestRank) { bestRank = rank; bestArc = name; bestMember = m; }
			}
		}
		if (bestMember.empty()) { note = "no model member found for race '" + code + "'"; return false; }

		std::vector<uint8_t> buf;
		if (!scanArc.Open(dir + bestArc) || !scanArc.Extract(bestMember, buf))
		{
			note = "open/decompress failed: " + bestMember + " in " + bestArc;
			return false;
		}
		isWld = ends(bestMember, ".wld");
		if (isWld)
		{
			if (!eqgfx::wld::ParseCharacter(buf.data(), buf.size(), code, model, err, &headBone))
			{
				note = "WLD parse fail " + bestMember + ": " + err;
				return false;
			}
		}
		else if (!eqgfx::eqg::ParseModel(buf.data(), buf.size(), model, err))
		{
			note = "parse fail " + bestMember + ": " + err;
			return false;
		}
	}

	// Body texture/model archive: global_chr for the original body, else the scanned
	// per-race archive.
	eqgfx::pfs::Archive& a = parsed ? globalChr : scanArc;

	// Old models wear the head as a separate mesh; graft the captured one on via
	// the body's head bone and drop the body's own head/face/eye submeshes so
	// there's no double head.
	std::string headMesh;
	{ std::scoped_lock lk(s_capMutex); headMesh = s_headMesh; }
	if (isWld && haveGlobalChr && headBone.valid && !headMesh.empty())
	{
		std::string want = headMesh;                 // "hafhe03_dmspritedef" -> "hafhe03"
		size_t suf = want.find("_dmspritedef");
		if (suf != std::string::npos) want.erase(suf);
		size_t bodySubmeshCount = model.submeshes.size();
		const float kD2R = 3.14159265f / 180.0f;
		float aYaw = s_headYawDeg.load() * kD2R, aPitch = s_headPitchDeg.load() * kD2R;
		float aRoll = s_headRollDeg.load() * kD2R, adz = s_headDzMilli.load() * 0.001f;
		std::vector<uint8_t> gbuf; std::string herr;
		if (globalChr.Extract("global_chr.wld", gbuf)
			&& eqgfx::wld::MergeHeadMesh(gbuf.data(), gbuf.size(), want, headBone, model, herr, aYaw, aPitch, aRoll, adz))
		{
			std::string codeHe = code + "he";        // e.g. "hafhe" (scalp/face)
			std::vector<eqgfx::eqg::SubMesh> kept;
			kept.reserve(model.submeshes.size());
			for (size_t si = 0; si < model.submeshes.size(); ++si)
			{
				const eqgfx::eqg::SubMesh& sm = model.submeshes[si];
				bool bodyHead = false;
				if (si < bodySubmeshCount && sm.materialIndex >= 0 && sm.materialIndex < (int)model.materials.size())
				{
					std::string mn = NormMat(model.materials[sm.materialIndex].name);
					bodyHead = (mn.compare(0, codeHe.size(), codeHe) == 0) || mn.find("eye") != std::string::npos;
				}
				if (!bodyHead) kept.push_back(sm);
			}
			model.submeshes.swap(kept);
		}
	}

	// Load each material's diffuse texture. Parallel to model.materials;
	// failures -> white.
	textures.assign(model.materials.size(), eqgfx::dds::Image{});
	for (size_t i = 0; i < model.materials.size(); ++i)
	{
		const std::string& tex = model.materials[i].diffuseTexture;
		if (tex.empty() || tex == "None") continue;
		std::vector<uint8_t> tbuf;
		if (a.Extract(tex, tbuf))
			eqgfx::dds::ParseTexture(tbuf.data(), tbuf.size(), textures[i]);
		if (!textures[i].valid() && haveGlobalChr && globalChr.Extract(tex, tbuf))
			eqgfx::dds::ParseTexture(tbuf.data(), tbuf.size(), textures[i]);
	}

	src = bestMember + " in " + bestArc;

	// Captured worn look first; equipment-type heuristic when no capture yet.
	if (ApplyCapturedAppearance(code, a, haveGlobalChr ? &globalChr : nullptr, model, textures))
	{
		if (equipReport)
		{
			std::scoped_lock lk(s_capMutex);
			*equipReport = "captured " + std::to_string(s_matEvents.size()) + " renames, head='"
				+ (s_headMesh.empty() ? "-" : s_headMesh) + "' dye=" + std::to_string(s_headTint);
		}
	}
	else
	{
		std::string rpt;
		ApplyEquipmentArmor(subj, code, dir, a, model, textures, rpt);
		if (equipReport) *equipReport = "no capture (zone to record) - heuristic: " + rpt;
	}

	// Optional Velious armor overlay (lower body).
	if (!armorType.empty())
	{
		int n = LoadArmorOverlay(dir, code, armorType, model, textures);
		src += n ? (" + " + std::to_string(n) + "pc " + armorType + " armor")
			: (" (no " + armorType + " armor file for " + code + ")");
	}

	return true;
}

} // namespace

// /eqgfx dumpchar - dump the subject's appearance state and equipped items.
void DumpCharCommand()
{
	PlayerClient* subj = pTarget.get();
	bool isSelf = false;
	if (!subj) { subj = pLocalPlayer.get(); isSelf = true; }
	if (!subj) { WriteChatf("\ag[MQEQGFX]\ax no target or self"); return; }

	auto& ac = subj->mActorClient;
	WriteChatf("\ag[MQEQGFX]\ax === appearance (%s) ===", isSelf ? "self" : "target");
	WriteChatf("  Race=%d RaceOverride=%d Class=%d Gender=%d ActorDef='%s'",
		ac.Race, ac.RaceOverride, ac.Class, (int)ac.Gender, ac.ActorDef);
	WriteChatf("  TextureType=%d Material=%d Variation=%d HeadType=%d FaceStyle=%d bShowHelm=%d",
		(int)ac.TextureType, (int)ac.Material, (int)ac.Variation, (int)ac.HeadType, (int)ac.FaceStyle, (int)ac.bShowHelm);
	WriteChatf("  Hair=%d HairColor=%d FacialHair=%d FHColor=%d Eye1=%d Eye2=%d Heritage=%d Tattoo=%d Details=%d",
		(int)ac.HairStyle, (int)ac.HairColor, (int)ac.FacialHair, (int)ac.FacialHairColor,
		(int)ac.EyeColor1, (int)ac.EyeColor2, ac.Heritage, ac.Tattoo, ac.Details);

	static const char* kName[9] = { "head", "chest", "arms", "wrist", "hands", "legs", "feet", "primary", "offhand" };
	for (int s = 0; s < 9; ++s)
	{
		auto& e = ac.ActorEquipment.Item[s];
		WriteChatf("  ActorEquip[%s] ty=%u var=%u mat=%u nid=%u nty=%u dye=%06X",
			kName[s], e.type, e.variation, e.material, e.newArmorID, e.newArmorType, ac.ArmorColor[s] & 0xFFFFFF);
	}

	PcProfile* prof = GetPcProfile();
	if (!prof) { WriteChatf("\ag[MQEQGFX]\ax (no PcProfile - items are self-only)"); return; }
	const int slots[9] = { InvSlot_Head, InvSlot_Chest, InvSlot_Arms, InvSlot_LeftWrist, InvSlot_Hands,
		InvSlot_Legs, InvSlot_Feet, InvSlot_Primary, InvSlot_Secondary };
	WriteChatf("\ag[MQEQGFX]\ax === equipped items (self inventory) ===");
	for (int i = 0; i < 9; ++i)
	{
		ItemPtr item = prof->GetInventorySlot(slots[i]);
		if (!item) { WriteChatf("  item[%s] (empty)", kName[i]); continue; }
		ItemDefinition* d = item->GetItemDefinition();
		if (!d) { WriteChatf("  item[%s] (no def)", kName[i]); continue; }
		auto& ap = d->ArmorProps;
		WriteChatf("  item[%s] '%s' id=%d IDFile=%d IDFile2=%d class=%d tintIdx=%u AP{ty=%u var=%u mat=%u nid=%u nty=%u}",
			kName[i], d->Name, d->ItemNumber, d->IDFile, d->IDFile2, (int)d->ItemClass, d->MaterialTintIndex,
			ap.type, ap.variation, ap.material, ap.newArmorID, ap.newArmorType);
	}
}

// /eqgfx dumpmodel - locate + parse the subject's model file and print its
// structure.
void DumpModelCommand()
{
	eqgfx::eqg::Model model;
	std::vector<eqgfx::dds::Image> textures;
	std::string src, note, equip;
	if (!LoadSubjectModel(model, textures, src, note, "", &equip))
	{
		WriteChatf("\ay[MQEQGFX]\ax %s", note.c_str());
		return;
	}
	if (!equip.empty())
		WriteChatf("\ag[MQEQGFX]\ax equip: %s", equip.c_str());
	int texLoaded = 0;
	for (const auto& t : textures) if (t.valid()) ++texLoaded;
	WriteChatf("\ag[MQEQGFX]\ax \agOK\ax %s: %d verts, %d tris, %d submeshes, %d materials, %d bones",
		src.c_str(), static_cast<int>(model.vertices.size()), static_cast<int>(model.indices.size() / 3),
		static_cast<int>(model.submeshes.size()), static_cast<int>(model.materials.size()), model.boneCount);
	WriteChatf("\ag[MQEQGFX]\ax bounds x[%.1f,%.1f] y[%.1f,%.1f] z[%.1f,%.1f]  textures %d/%d loaded",
		model.minB[0], model.maxB[0], model.minB[1], model.maxB[1], model.minB[2], model.maxB[2],
		texLoaded, static_cast<int>(model.materials.size()));
	for (size_t i = 0; i < model.materials.size() && i < 5; ++i)
		WriteChatf("\ag[MQEQGFX]\ax   mat[%d] '%s' tex='%s' %s", static_cast<int>(i),
			model.materials[i].name.c_str(), model.materials[i].diffuseTexture.c_str(),
			(i < textures.size() && textures[i].valid()) ? "\ag[ok]\ax" : "");
}

// ---- Live capture of the game's own appearance calls -----------------------
// Detour the actor's appearance calls (addresses read from the local actor's
// vtable) and record what the game passes for the local player.

namespace {

// True only for the local player's actor.
bool IsLocalActor(void* self)
{
	PlayerClient* me = pLocalPlayer.get();
	return me && static_cast<void*>(me->mActorClient.pActor) == self;
}

// Record one ReplaceMaterial(a,b,tint) rename onto the ordered stream.
void RecordReplaceMaterial(const char* a, const char* b, uint32_t tint)
{
	if (!a || !b) return;
	std::scoped_lock lk(s_capMutex);
	if (s_matEvents.size() >= kMaxMatEvents)
		s_matEvents.erase(s_matEvents.begin(), s_matEvents.begin() + s_matEvents.size() / 4);
	s_matEvents.push_back({ NormMat(a), NormMat(b), tint & 0xFFFFFFu });
	s_haveCapture = true;
	s_captureDirty.store(true, std::memory_order_relaxed);
	if (PlayerClient* me = pLocalPlayer.get()) s_captureActorDef = me->mActorClient.ActorDef;
}

// Record the chosen worn head + its dye.
void RecordHead(const char* mesh, uint32_t tint)
{
	std::scoped_lock lk(s_capMutex);
	if (mesh && *mesh)
	{
		std::string m = mesh;
		for (char& c : m) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		s_headMesh = m;
	}
	s_headTint = tint & 0xFFFFFFu;
	s_haveCapture = true;
	s_captureDirty.store(true, std::memory_order_relaxed);
}

class ActorSwapHook
{
public:
	// ReplaceMaterial(nameA, nameB, tint, flag, nameC, tint2): the per-slot
	// armor material swap.
	DETOUR_TRAMPOLINE_DEF(bool, ReplaceMaterial_Trampoline, (const char*, const char*, uint32_t*, char, const char*, uint32_t*))
	bool ReplaceMaterial_Detour(const char* a, const char* b, uint32_t* tint, char flag, const char* c, uint32_t* tint2)
	{
		if (IsLocalActor(this))
		{
			uint32_t t = tint ? (*tint & 0xFFFFFF) : 0;
			RecordReplaceMaterial(a, b, t);
			if (s_captureVerbose)
				WriteChatf("\ag[MQEQGFX]\ax \at>>> ReplaceMaterial\ax(a='%s' b='%s' c='%s' tint=%06X tint2=%06X flag=%d)",
					a ? a : "-", b ? b : "-", c ? c : "-",
					t, tint2 ? (*tint2 & 0xFFFFFF) : 0, (int)flag);
		}
		return ReplaceMaterial_Trampoline(a, b, tint, flag, c, tint2);
	}

	DETOUR_TRAMPOLINE_DEF(bool, SwapBody_Trampoline, (int, const char*))
	bool SwapBody_Detour(int bodyType, const char* material)
	{
		if (IsLocalActor(this) && s_captureVerbose)
			WriteChatf("\ag[MQEQGFX]\ax \at>>> SwapBody\ax(bodyType=%d, material='%s')",
				bodyType, material ? material : "(null)");
		return SwapBody_Trampoline(bodyType, material);
	}

	// p3 carries the dye tint (0xAARRGGBB), helmGfx the helm graphic.
	DETOUR_TRAMPOLINE_DEF(bool, SwapHead_Trampoline, (const char*, void*, void*, char, char, uint32_t))
	bool SwapHead_Detour(const char* headMesh, void* p3, void* p4, char p5, char p6, uint32_t helmGfx)
	{
		if (IsLocalActor(this))
		{
			uint32_t tint = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p3)) & 0xFFFFFF;
			RecordHead(headMesh, tint);
			if (s_captureVerbose)
				WriteChatf("\ag[MQEQGFX]\ax \at>>> SwapHead\ax(mesh='%s', tint=%06X flags=%d,%d, helmGfx=%u)",
					headMesh ? headMesh : "(null)", tint, (int)p5, (int)p6, helmGfx);
		}
		return SwapHead_Trampoline(headMesh, p3, p4, p5, p6, helmGfx);
	}

	DETOUR_TRAMPOLINE_DEF(bool, SwapMaterials_Trampoline, (const char*))
	bool SwapMaterials_Detour(const char* matSet)
	{
		if (IsLocalActor(this) && s_captureVerbose)
			WriteChatf("\ag[MQEQGFX]\ax \at>>> SwapMaterials\ax(set='%s')", matSet ? matSet : "(null)");
		return SwapMaterials_Trampoline(matSet);
	}
};

// CActor vtable byte offsets.
constexpr size_t kVtReplaceMaterial = 0x1d0;
constexpr size_t kVtSwapBody        = 0x290;
constexpr size_t kVtSwapHead        = 0x2a0;
constexpr size_t kVtSwapMaterials   = 0x4a0;

uintptr_t s_replaceMatAddr = 0;
uintptr_t s_swapBodyAddr = 0;
uintptr_t s_swapHeadAddr = 0;
uintptr_t s_swapMatAddr = 0;

// Persisted capture: <MQ config>/eqgfx_look_<actordef>.txt, one "MAT a b tint"
// line per rename.
std::string CaptureFilePath(const std::string& actorDef)
{
	std::string tag = actorDef.empty() ? "unknown" : actorDef;
	for (char& c : tag) { if (!std::isalnum(static_cast<unsigned char>(c))) c = '_'; else c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
	return std::string(gPathConfig) + "/eqgfx_look_" + tag + ".txt";
}

} // namespace

// Detour the 4 appearance calls on the local actor's vtable. Silent when
// auto_=true. Returns true if the detours are now live.
bool InstallSwapCapture(bool auto_)
{
	if (s_captureInstalled)
	{
		if (!auto_) WriteChatf("\ag[MQEQGFX]\ax capture already ON");
		return true;
	}
	PlayerClient* me = pLocalPlayer.get();
	if (!me || !me->mActorClient.pActor)
	{
		if (!auto_) WriteChatf("\ay[MQEQGFX]\ax no local actor - must be standing in the world");
		return false;
	}
	uint8_t* vtbl = *reinterpret_cast<uint8_t**>(me->mActorClient.pActor);
	s_replaceMatAddr = *reinterpret_cast<uintptr_t*>(vtbl + kVtReplaceMaterial);
	s_swapBodyAddr   = *reinterpret_cast<uintptr_t*>(vtbl + kVtSwapBody);
	s_swapHeadAddr   = *reinterpret_cast<uintptr_t*>(vtbl + kVtSwapHead);
	s_swapMatAddr    = *reinterpret_cast<uintptr_t*>(vtbl + kVtSwapMaterials);
	EzDetour(s_replaceMatAddr, &ActorSwapHook::ReplaceMaterial_Detour, &ActorSwapHook::ReplaceMaterial_Trampoline);
	EzDetour(s_swapBodyAddr,   &ActorSwapHook::SwapBody_Detour,        &ActorSwapHook::SwapBody_Trampoline);
	EzDetour(s_swapHeadAddr,   &ActorSwapHook::SwapHead_Detour,        &ActorSwapHook::SwapHead_Trampoline);
	EzDetour(s_swapMatAddr,    &ActorSwapHook::SwapMaterials_Detour,   &ActorSwapHook::SwapMaterials_Trampoline);
	s_captureInstalled = true;
	if (!auto_)
		WriteChatf("\ag[MQEQGFX]\ax capture \agON\ax (verbose). Zone or re-equip to log every appearance call.");
	return true;
}

void RemoveSwapCapture()
{
	if (s_replaceMatAddr) { RemoveDetour(s_replaceMatAddr); s_replaceMatAddr = 0; }
	if (s_swapBodyAddr)   { RemoveDetour(s_swapBodyAddr);   s_swapBodyAddr = 0; }
	if (s_swapHeadAddr)   { RemoveDetour(s_swapHeadAddr);   s_swapHeadAddr = 0; }
	if (s_swapMatAddr)    { RemoveDetour(s_swapMatAddr);    s_swapMatAddr = 0; }
	s_captureInstalled = false;
}

void SaveCapture()
{
	std::vector<MatEvent> events; std::string head, actorDef; uint32_t htint;
	{
		std::scoped_lock lk(s_capMutex);
		if (!s_haveCapture) return;
		events = s_matEvents; head = s_headMesh; htint = s_headTint; actorDef = s_captureActorDef;
	}
	FILE* f = fopen(CaptureFilePath(actorDef).c_str(), "w");
	if (!f) return;
	fprintf(f, "HEAD %s %06X\n", head.empty() ? "-" : head.c_str(), htint);
	for (const MatEvent& e : events) fprintf(f, "MAT %s %s %06X\n", e.a.c_str(), e.b.c_str(), e.tint);
	fclose(f);
	s_captureDirty.store(false, std::memory_order_relaxed);
}

// Load a persisted capture for 'actorDef' into the live state.
bool LoadCapture(const std::string& actorDef)
{
	FILE* f = fopen(CaptureFilePath(actorDef).c_str(), "r");
	if (!f) return false;
	std::vector<MatEvent> events; std::string head; uint32_t htint = 0;
	char kind[16], a[128], b[128]; unsigned t;
	while (true)
	{
		int n = fscanf(f, "%15s", kind);
		if (n != 1) break;
		if (std::strcmp(kind, "HEAD") == 0) { if (fscanf(f, "%127s %x", a, &t) == 2) { head = (std::strcmp(a, "-") == 0) ? "" : a; htint = t; } }
		else if (std::strcmp(kind, "MAT") == 0) { if (fscanf(f, "%127s %127s %x", a, b, &t) == 3) events.push_back({ a, b, t }); }
	}
	fclose(f);
	std::scoped_lock lk(s_capMutex);
	s_matEvents = std::move(events);
	s_headMesh = head; s_headTint = htint; s_captureActorDef = actorDef;
	s_haveCapture = true;
	return true;
}

// (Re)load the subject model and hand it to the render thread. Main thread
// only (file I/O + game reads).
bool BuildModelView(const std::string& armor, std::string& src, std::string& note)
{
	eqgfx::eqg::Model m;
	std::vector<eqgfx::dds::Image> tex;
	if (!LoadSubjectModel(m, tex, src, note, armor)) return false;
	std::scoped_lock lock(s_modelMutex);
	s_pendingModel = std::move(m);
	s_pendingTextures = std::move(tex);
	s_modelDirty = true;
	return true;
}

// ---- Skeleton posing -------------------------------------------------------
// Re-skin a parsed WLD model (bone-local verts + skeleton) into a new world pose
// given a per-bone Euler adjustment (degrees). With all-zero adjustments this
// reproduces the baked bind pose exactly. Render-thread only.
namespace pose {
inline void Mul3(const float* A, const float* B, float* o) {
	for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
		o[i * 3 + j] = A[i * 3] * B[j] + A[i * 3 + 1] * B[3 + j] + A[i * 3 + 2] * B[6 + j];
}
inline void Vec3(const float* A, const float* v, float* o) {
	for (int i = 0; i < 3; ++i) o[i] = A[i * 3] * v[0] + A[i * 3 + 1] * v[1] + A[i * 3 + 2] * v[2];
}
inline void EulerDeg(float rx, float ry, float rz, float* R) {
	const float d = 3.14159265f / 180.0f; rx *= d; ry *= d; rz *= d;
	float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	float Rx[9] = { 1, 0, 0, 0, cx, -sx, 0, sx, cx };
	float Ry[9] = { cy, 0, sy, 0, 1, 0, -sy, 0, cy };
	float Rz[9] = { cz, -sz, 0, sz, cz, 0, 0, 0, 1 };
	float t[9]; Mul3(Rz, Ry, t); Mul3(t, Rx, R);   // Rz*Ry*Rx
}
} // namespace pose

bool ModelIsPoseable(const eqgfx::eqg::Model& m)
{
	return !m.skeleton.empty() && m.localVertices.size() == m.vertices.size()
		&& m.vertexBone.size() == m.vertices.size();
}

// out = base re-skinned by adjDeg (per-bone Euler, parallel to skeleton).
void SkinModel(const eqgfx::eqg::Model& base, const std::vector<std::array<float, 3>>& adjDeg,
	eqgfx::eqg::Model& out)
{
	out = base;   // copy indices/submeshes/materials; overwrite vertices below
	const size_t nb = base.skeleton.size();
	std::vector<std::array<float, 9>> wR(nb);
	std::vector<std::array<float, 3>> wT(nb);
	std::vector<char> done(nb, 0);
	// Forward kinematics: process a bone once its parent is done (repeat until stable).
	bool progress = true;
	while (progress)
	{
		progress = false;
		for (size_t b = 0; b < nb; ++b)
		{
			if (done[b]) continue;
			int p = base.skeleton[b].parent;
			if (p >= 0 && !done[p]) continue;
			float adj[9] = { 1,0,0, 0,1,0, 0,0,1 };
			if (b < adjDeg.size()) pose::EulerDeg(adjDeg[b][0], adjDeg[b][1], adjDeg[b][2], adj);
			float eff[9]; pose::Mul3(base.skeleton[b].localR, adj, eff);   // localR * adj
			if (p < 0)
			{
				std::memcpy(wR[b].data(), eff, sizeof(eff));
				std::memcpy(wT[b].data(), base.skeleton[b].localT, sizeof(float) * 3);
			}
			else
			{
				pose::Mul3(wR[p].data(), eff, wR[b].data());
				float rt[3]; pose::Vec3(wR[p].data(), base.skeleton[b].localT, rt);
				wT[b][0] = rt[0] + wT[p][0]; wT[b][1] = rt[1] + wT[p][1]; wT[b][2] = rt[2] + wT[p][2];
			}
			done[b] = 1; progress = true;
		}
	}
	for (int k = 0; k < 3; ++k) { out.minB[k] = 1e30f; out.maxB[k] = -1e30f; }
	for (size_t i = 0; i < out.vertices.size(); ++i)
	{
		const eqgfx::eqg::Vertex& lv = base.localVertices[i];
		eqgfx::eqg::Vertex& wv = out.vertices[i];
		int b = base.vertexBone[i];
		if (b >= 0 && static_cast<size_t>(b) < nb && done[b])
		{
			float lp[3] = { lv.px, lv.py, lv.pz }, wp[3]; pose::Vec3(wR[b].data(), lp, wp);
			wv.px = wp[0] + wT[b][0]; wv.py = wp[1] + wT[b][1]; wv.pz = wp[2] + wT[b][2];
			float ln[3] = { lv.nx, lv.ny, lv.nz }, wn[3]; pose::Vec3(wR[b].data(), ln, wn);
			wv.nx = wn[0]; wv.ny = wn[1]; wv.nz = wn[2];
		}
		else { wv.px = lv.px; wv.py = lv.py; wv.pz = lv.pz; wv.nx = lv.nx; wv.ny = lv.ny; wv.nz = lv.nz; }
		wv.u = lv.u; wv.v = lv.v;
		const float p[3] = { wv.px, wv.py, wv.pz };
		for (int k = 0; k < 3; ++k) { if (p[k] < out.minB[k]) out.minB[k] = p[k]; if (p[k] > out.maxB[k]) out.maxB[k] = p[k]; }
	}
}

// The model window: renders the parsed model to an offscreen texture and shows
// it with ImGui::Image. Render thread.
PLUGIN_API void OnUpdateImGui()
{
	if (!s_showModel.load(std::memory_order_relaxed))
		return;
	if (!gbDeviceAcquired || gpD3D11Device == nullptr)
		return;
	if (!s_modelView.EnsureInit(gpD3D11Device, 384, 512))
		return;
	static eqgfx::eqg::Model rtBase;
	static std::vector<eqgfx::dds::Image> rtTex;
	static std::vector<std::array<float, 3>> boneAdj;
	static int selBone = -1;
	bool newModel = false;
	{
		std::scoped_lock lock(s_modelMutex);
		if (s_modelDirty)
		{
			rtBase = std::move(s_pendingModel);
			rtTex = std::move(s_pendingTextures);
			s_modelDirty = false; newModel = true;
		}
	}
	if (newModel)
	{
		boneAdj.assign(rtBase.skeleton.size(), std::array<float, 3>{ 0, 0, 0 });
		selBone = -1;
		s_modelView.SetModel(rtBase, rtTex);   // bind pose
	}
	if (!s_modelView.HasModel())
		return;

	// Orbit camera: left-drag = rotate, right-drag = pan, wheel = zoom.
	static float yaw = 0.7f, pitch = 0.15f, zoom = 1.0f, panX = 0.0f, panY = 0.0f;
	static bool  spin = false;
	static bool  breathe = true;
	static float breathRate = 3.2f, breathDepth = 1.0f;
	static float animTime = 0.0f;
	animTime += ImGui::GetIO().DeltaTime;
	if (spin) yaw += 0.01f;
	s_modelView.RenderFrame(yaw, pitch, zoom, panX, panY, animTime, breathe, breathRate, breathDepth);

	bool open = true;
	if (ImGui::Begin("EQGFX Model", &open))
	{
		ImGui::Image((ImTextureID)s_modelView.TextureId(),
			ImVec2(static_cast<float>(s_modelView.Width()), static_cast<float>(s_modelView.Height())));

		if (ImGui::IsItemHovered())
		{
			ImGuiIO& io = ImGui::GetIO();
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				yaw   -= d.x * 0.01f;
				pitch += d.y * 0.01f;
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			}
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			{
				ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
				panX -= d.x * 0.004f;
				panY += d.y * 0.004f;
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
			}
			if (io.MouseWheel != 0.0f)
				zoom *= (1.0f + io.MouseWheel * 0.1f);
		}
		if (pitch >  1.5f) pitch =  1.5f;
		if (pitch < -1.5f) pitch = -1.5f;
		if (zoom  <  0.2f) zoom  =  0.2f;
		if (zoom  >  8.0f) zoom  =  8.0f;

		ImGui::Checkbox("spin", &spin);
		ImGui::SameLine();
		ImGui::Checkbox("breathe", &breathe);
		ImGui::SameLine();
		if (ImGui::Button("reset view")) { yaw = 0.7f; pitch = 0.15f; zoom = 1.0f; panX = 0.0f; panY = 0.0f; }
		ImGui::SameLine();
		ImGui::TextDisabled("drag=rotate  R-drag=pan  wheel=zoom");
		if (breathe)
		{
			ImGui::SliderFloat("breath speed", &breathRate, 0.5f, 12.0f, "%.1f");
			ImGui::SliderFloat("breath depth", &breathDepth, 0.0f, 4.0f, "%.1f");
		}

		if (ImGui::CollapsingHeader("head placement"))
		{
			int hy = s_headYawDeg.load(), hp = s_headPitchDeg.load(), hr = s_headRollDeg.load(), hz = s_headDzMilli.load();
			bool changed = false;
			changed |= ImGui::SliderInt("yaw",   &hy, -180, 180);
			changed |= ImGui::SliderInt("pitch", &hp, -180, 180);
			changed |= ImGui::SliderInt("roll",  &hr, -180, 180);
			changed |= ImGui::SliderInt("height (mZ)", &hz, -500, 500);
			if (ImGui::Button("reset head")) { hy = 0; hp = 0; hr = 180; hz = 500; changed = true; }
			if (changed)
			{
				s_headYawDeg.store(hy); s_headPitchDeg.store(hp);
				s_headRollDeg.store(hr); s_headDzMilli.store(hz);
				s_reloadModel.store(true);
			}
			ImGui::TextDisabled("yaw %d  pitch %d  roll %d  z %d", hy, hp, hr, hz);
		}

		if (!rtBase.skeleton.empty() && ImGui::CollapsingHeader("pose skeleton"))
		{
			bool poseable = ModelIsPoseable(rtBase);
			if (!poseable) ImGui::TextDisabled("(not poseable for this model)");
			const char* cur = (selBone >= 0 && selBone < (int)rtBase.skeleton.size())
				? rtBase.skeleton[selBone].name.c_str() : "(select bone)";
			if (ImGui::BeginCombo("bone", cur))
			{
				for (int b = 0; b < (int)rtBase.skeleton.size(); ++b)
					if (ImGui::Selectable(rtBase.skeleton[b].name.c_str(), b == selBone)) selBone = b;
				ImGui::EndCombo();
			}
			bool changed = false;
			if (selBone >= 0 && selBone < (int)boneAdj.size())
			{
				auto& e = boneAdj[selBone];
				changed |= ImGui::SliderFloat("rot X", &e[0], -180.0f, 180.0f, "%.0f");
				changed |= ImGui::SliderFloat("rot Y", &e[1], -180.0f, 180.0f, "%.0f");
				changed |= ImGui::SliderFloat("rot Z", &e[2], -180.0f, 180.0f, "%.0f");
				if (ImGui::Button("reset bone")) { e = std::array<float, 3>{ 0, 0, 0 }; changed = true; }
				ImGui::SameLine();
			}
			if (ImGui::Button("reset pose")) { for (auto& a : boneAdj) a = std::array<float, 3>{ 0, 0, 0 }; changed = true; }
			if (changed && poseable)
			{
				eqgfx::eqg::Model posed;
				SkinModel(rtBase, boneAdj, posed);
				s_modelView.UpdatePose(posed.vertices);   // geometry-only; keeps textures + framing
			}
		}
	}
	ImGui::End();
	if (!open)
		s_showModel.store(false);
}
