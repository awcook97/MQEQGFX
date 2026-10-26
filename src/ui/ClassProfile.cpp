#include "ClassProfile.h"
#include "HotButtons.h"

#include <mq/Plugin.h>

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <unordered_map>

namespace eqgfx::ui {

using settings::json;

namespace {

struct Placement
{
	ImVec2 pos;
	ImVec2 size;
	bool   pending = false;
};

std::string    s_class;
settings::File s_file;
int            s_serial = 0;
std::unordered_map<std::string, Placement> s_windows;

std::string ClassNow()
{
	const PlayerClient* me = pLocalPlayer.get();
	if (!me || GetGameState() != GAMESTATE_INGAME)
		return {};
	const int cls = me->GetClass();
	if (cls <= 0 || cls >= static_cast<int>(std::size(ClassInfo)))
		return {};
	return ClassInfo[cls].ShortName;
}

json& Node(json& parent, const char* key)
{
	json& node = parent[key];
	if (!node.is_object())
		node = json::object();
	return node;
}

void LoadWindows()
{
	s_windows.clear();
	const json& windows = Node(s_file.Root(), "Windows");
	for (const auto& [id, v] : windows.items())
	{
		if (!v.is_array() || v.size() < 2)
			continue;
		Placement pl;
		pl.pos = ImVec2(settings::AsFloat(v[0], 0.0f), settings::AsFloat(v[1], 0.0f));
		if (v.size() >= 4)
			pl.size = ImVec2(settings::AsFloat(v[2], 0.0f), settings::AsFloat(v[3], 0.0f));
		pl.pending = true;
		s_windows[id] = pl;
	}
}

bool StartsWithCi(std::string_view s, std::string_view prefix)
{
	return s.size() >= prefix.size() && ci_equals(s.substr(0, prefix.size()), prefix);
}

json ImportClassIni(const settings::IniFile& ini)
{
	json root = json::object();
	json& options = Node(root, "Options");
	json bars = json::array();
	for (int bar = 0; bar < kHotBars; ++bar)
	{
		json pages = json::array();
		for (int page = 0; page < kHotPages; ++page)
			pages.push_back(json::array());
		bars.push_back(std::move(pages));
	}

	for (const auto& [section, keys] : ini)
	{
		int bar = 0, page = 0;
		if (ci_equals(section, "Windows"))
		{
			json& windows = Node(root, "Windows");
			for (const auto& [id, value] : keys)
			{
				int x = 0, y = 0, w = 0, h = 0;
				if (std::sscanf(value.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h) >= 2)
					windows[id] = json::array({ x, y, w, h });
			}
		}
		else if (ci_equals(section, "Layout"))
		{
			json& layout = Node(root, "Layout");
			for (const auto& [id, value] : keys)
			{
				float size = 40.0f, spacing = 2.0f, opacity = 1.0f;
				int columns = -1, labels = 1, showKeys = 1, locked = 0;
				if (std::sscanf(value.c_str(), "%f,%f,%d,%f,%d,%d,%d", &size, &spacing, &columns, &opacity,
					&labels, &showKeys, &locked) >= 4)
				{
					layout[id] = { { "size", size }, { "spacing", spacing }, { "columns", columns },
						{ "opacity", opacity }, { "labels", labels != 0 }, { "keys", showKeys != 0 },
						{ "locked", locked != 0 } };
				}
			}
		}
		else if (ci_equals(section, "Chat"))
		{
			std::vector<std::pair<int, json>> tabs;
			for (const auto& [key, value] : keys)
			{
				const size_t comma = value.find(',');
				if (!StartsWithCi(key, "Tab") || comma == std::string::npos)
					continue;
				tabs.emplace_back(std::atoi(key.c_str() + 3), json{ { "name", value.substr(comma + 1) },
					{ "mask", std::strtoul(value.c_str(), nullptr, 10) } });
			}
			std::sort(tabs.begin(), tabs.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
			json list = json::array();
			for (auto& [n, tab] : tabs)
				list.push_back(std::move(tab));
			Node(root, "Chat")["Tabs"] = std::move(list);
		}
		else if (std::sscanf(section.c_str(), "Bar%d.Page%d", &bar, &page) == 2
			|| std::sscanf(section.c_str(), "bar%d.page%d", &bar, &page) == 2)
		{
			if (bar < 1 || bar > kHotBars || page < 1 || page > kHotPages)
				continue;
			json slots = json::array();
			for (int s = 0; s < kHotSlots; ++s)
				slots.push_back(nullptr);
			for (const auto& [key, value] : keys)
			{
				const int slot = std::atoi(key.c_str()) - 1;
				if (slot >= 0 && slot < kHotSlots)
					slots[slot] = ButtonJson(ButtonFromLegacy(value));
			}
			bars[bar - 1][page - 1] = std::move(slots);
		}
		else if (StartsWithCi(section, "Set."))
		{
			int count = -1;
			std::vector<std::pair<int, json>> read;
			for (const auto& [key, value] : keys)
			{
				if (ci_equals(key, "Count"))
					count = GetIntFromString(value, -1);
				else
					read.emplace_back(std::atoi(key.c_str()) - 1, ButtonJson(ButtonFromLegacy(value)));
			}
			if (count < 0)
				continue;
			json list = json::array();
			for (int i = 0; i < count; ++i)
				list.push_back(nullptr);
			for (auto& [slot, b] : read)
				if (slot >= 0 && slot < count)
					list[slot] = std::move(b);
			Node(root, "ButtonSets")[section.substr(4)] = std::move(list);
		}
		else if (ci_equals(section, "Profile"))
		{
			for (const auto& [key, value] : keys)
				if (ci_equals(key, "Imported"))
					root["ImportedEqBars"] = value == "1";
		}
		else
		{
			json& values = Node(options, section.c_str());
			for (const auto& [key, value] : keys)
				values[key] = settings::FromIniValue(value);
		}
	}
	root["HotBars"] = std::move(bars);
	return root;
}

} // namespace

const std::string& ProfileClass() { return s_class; }
int ProfileSerial()               { return s_serial; }

void UpdateProfile()
{
	const std::string cls = ClassNow();
	if (cls != s_class)
	{
		s_file.Close();
		s_class = cls;
		++s_serial;
		s_windows.clear();
		if (!cls.empty())
		{
			const std::filesystem::path folder = settings::ClassesDir();
			s_file.Open(folder / (cls + ".json"), folder / (cls + ".ini"), ImportClassIni);
			LoadWindows();
		}
	}
	s_file.Pulse();
}

void FlushProfile()
{
	if (s_file.IsOpen())
		s_file.Save();
}

static ImVec2 OnScreen(ImVec2 pos, ImVec2 size)
{
	const float w = size.x > 0.0f ? size.x : 64.0f;
	const float h = size.y > 0.0f ? size.y : 64.0f;
	const ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
	if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && !pio.Monitors.empty())
	{
		for (const ImGuiPlatformMonitor& m : pio.Monitors)
		{
			if (pos.x < m.WorkPos.x + m.WorkSize.x && pos.x + w > m.WorkPos.x
				&& pos.y < m.WorkPos.y + m.WorkSize.y && pos.y + h > m.WorkPos.y)
				return pos;
		}
	}
	const ImGuiViewport* vp = ImGui::GetMainViewport();
	if (vp->Size.x <= 0.0f || vp->Size.y <= 0.0f)
		return pos;
	pos.x = std::clamp(pos.x, vp->Pos.x, vp->Pos.x + vp->Size.x - std::min(w, vp->Size.x));
	pos.y = std::clamp(pos.y, vp->Pos.y, vp->Pos.y + vp->Size.y - std::min(h, vp->Size.y));
	return pos;
}

void PlaceWindow(const char* id, bool withSize)
{
	if (s_class.empty())
		return;
	auto it = s_windows.find(id);
	if (it != s_windows.end() && it->second.pending)
	{
		const ImVec2 size = it->second.size;
		ImGui::SetNextWindowPos(OnScreen(it->second.pos, withSize ? size : ImVec2(0.0f, 0.0f)), ImGuiCond_Always);
		if (withSize && size.x > 0.0f && size.y > 0.0f)
			ImGui::SetNextWindowSize(size, ImGuiCond_Always);
		it->second.pending = false;
	}
}

void NoteWindow(const char* id)
{
	if (s_class.empty())
		return;
	const ImVec2 pos = ImGui::GetWindowPos();
	const ImVec2 size = ImGui::GetWindowSize();
	auto [it, added] = s_windows.try_emplace(id);
	Placement& pl = it->second;
	if (pl.pending)
		return;
	if (added || std::fabs(pl.pos.x - pos.x) > 0.5f || std::fabs(pl.pos.y - pos.y) > 0.5f
		|| std::fabs(pl.size.x - size.x) > 0.5f || std::fabs(pl.size.y - size.y) > 0.5f)
	{
		pl.pos = pos;
		pl.size = size;
		Node(s_file.Root(), "Windows")[id] = json::array({ static_cast<int>(pos.x), static_cast<int>(pos.y),
			static_cast<int>(size.x), static_cast<int>(size.y) });
		s_file.MarkDirty();
	}
}

float ClassValue(const char* section, const char* key, float fallback)
{
	if (!s_file.IsOpen())
		return fallback;
	const json& root = s_file.Root();
	auto options = root.find("Options");
	if (options == root.end() || !options->is_object())
		return fallback;
	auto sec = options->find(section);
	if (sec == options->end() || !sec->is_object())
		return fallback;
	auto it = sec->find(key);
	return it != sec->end() ? settings::AsFloat(*it, fallback) : fallback;
}

void SetClassValue(const char* section, const char* key, float value)
{
	if (!s_file.IsOpen())
		return;
	Node(Node(s_file.Root(), "Options"), section)[key] = value;
	s_file.MarkDirty();
}

json& ProfileRoot()
{
	return s_file.Root();
}

void MarkProfileDirty()
{
	s_file.MarkDirty();
}

void ReloadProfile()
{
	if (s_class.empty())
		return;
	++s_serial;
	LoadWindows();
	s_file.MarkDirty();
}

} // namespace eqgfx::ui
