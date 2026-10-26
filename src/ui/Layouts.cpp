#include "Layouts.h"

#include "ClassProfile.h"
#include "Suppress.h"

#include "../MQEQGFX.h"
#include "../SettingsFile.h"
#include "../unitframes/UnitFrames.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iterator>
#include <system_error>

namespace eqgfx::ui {

using settings::json;

namespace {

std::filesystem::path Folder()
{
	return settings::LayoutsDir();
}

std::string Clean(const std::string& name)
{
	std::string out;
	for (char c : name)
		out += std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_' ? c : '_';
	while (!out.empty() && out.back() == ' ')
		out.pop_back();
	while (!out.empty() && out.front() == ' ')
		out.erase(out.begin());
	return out;
}

std::filesystem::path FileFor(const std::string& name)
{
	return Folder() / (Clean(name) + ".json");
}

bool Travels(std::string_view key, bool withButtons)
{
	if (key == "ImportedEqBars")
		return false;
	return withButtons || (key != "HotBars" && key != "ButtonSets");
}

void Say(const char* fmt, const std::string& name)
{
	WriteChatf(fmt, name.c_str());
}

} // namespace

std::vector<std::string> LayoutNames()
{
	std::vector<std::string> names;
	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(Folder(), ec))
		if (entry.is_regular_file(ec) && ci_equals(entry.path().extension().string(), ".json"))
			names.push_back(entry.path().stem().string());
	std::sort(names.begin(), names.end(), [](const std::string& a, const std::string& b) { return ci_less()(a, b); });
	return names;
}

bool SaveLayout(const std::string& name, bool withButtons)
{
	if (Clean(name).empty() || ProfileClass().empty())
		return false;

	unitframes::SaveSettings();

	json profile = json::object();
	for (const auto& [key, value] : ProfileRoot().items())
		if (Travels(key, withButtons))
			profile[key] = value;

	json layout = {
		{ "Class", ProfileClass() },
		{ "Buttons", withButtons },
		{ "Profile", std::move(profile) },
		{ "Replace", ReplaceList() },
	};
	if (const json* frames = &settings::Global().Root()["UnitFrames"]; frames->is_object())
		layout["UnitFrames"] = *frames;

	settings::WriteJson(FileFor(name), layout);
	Say("\ag[MQEQGFX]\ax saved layout \ay%s\ax", Clean(name));
	return true;
}

bool LoadLayout(const std::string& name)
{
	if (ProfileClass().empty())
		return false;
	const json layout = settings::ReadJson(FileFor(name));
	if (!layout.is_object())
	{
		Say("\ag[MQEQGFX]\ax no layout named \ay%s\ax", name);
		return false;
	}
	const auto profile = layout.find("Profile");
	if (profile == layout.end() || !profile->is_object())
	{
		Say("\ag[MQEQGFX]\ax \ay%s\ax is not a layout", name);
		return false;
	}
	const auto buttons = layout.find("Buttons");
	const bool withButtons = buttons != layout.end() && settings::AsBool(*buttons, false);

	json& root = ProfileRoot();
	for (auto it = root.begin(); it != root.end(); )
		it = Travels(it.key(), withButtons) ? root.erase(it) : std::next(it);
	for (const auto& [key, value] : profile->items())
		if (Travels(key, withButtons))
			root[key] = value;
	ReloadProfile();

	if (const auto frames = layout.find("UnitFrames"); frames != layout.end() && frames->is_object())
	{
		settings::Global().SetSection("UnitFrames", *frames);
		unitframes::LoadSettings();
	}

	if (const auto replace = layout.find("Replace"); replace != layout.end())
		ApplyReplaceList(settings::AsString(*replace, ""));

	Say("\ag[MQEQGFX]\ax loaded layout \ay%s\ax", Clean(name));
	return true;
}

bool DeleteLayout(const std::string& name)
{
	std::error_code ec;
	const bool gone = std::filesystem::remove(FileFor(name), ec);
	if (gone)
		Say("\ag[MQEQGFX]\ax deleted layout \ay%s\ax", Clean(name));
	return gone;
}

void DrawLayoutOptions()
{
	static char s_name[64] = {};
	static bool s_withButtons = false;
	static std::vector<std::string> s_names;
	static int s_serial = -1;
	static double s_listed = -10.0;
	if (s_serial != ProfileSerial() || ImGui::GetTime() - s_listed > 5.0)
	{
		s_serial = ProfileSerial();
		s_listed = ImGui::GetTime();
		s_names = LayoutNames();
	}

	ImGui::PushID("layouts");
	ImGui::TextWrapped("A layout keeps where every window is and how big, the bars' layouts, window options, the "
		"unit frames and which stock windows are ours. It loads onto any class; hot buttons go along only "
		"if saved with them.");
	if (ProfileClass().empty())
	{
		ImGui::TextDisabled("in game only");
		ImGui::PopID();
		return;
	}

	ImGui::SetNextItemWidth(180.0f);
	const bool enter = ImGui::InputTextWithHint("##name", "layout name", s_name, sizeof(s_name),
		ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SameLine();
	ImGui::BeginDisabled(Clean(s_name).empty());
	if ((ImGui::Button("save") || enter) && !Clean(s_name).empty())
	{
		SaveLayout(s_name, s_withButtons);
		s_listed = -10.0;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Checkbox("with hot buttons", &s_withButtons);

	if (s_names.empty())
		ImGui::TextDisabled("no layouts saved yet");
	for (const std::string& n : s_names)
	{
		ImGui::PushID(n.c_str());
		if (ImGui::SmallButton("load"))
			LoadLayout(n);
		ImGui::SameLine();
		if (ImGui::SmallButton("save over"))
			SaveLayout(n, s_withButtons);
		ImGui::SameLine();
		if (ImGui::SmallButton("delete"))
			ImGui::OpenPopup("##confirm");
		ImGui::SameLine();
		ImGui::TextUnformatted(n.c_str());
		if (ImGui::BeginPopup("##confirm"))
		{
			ImGui::Text("delete %s?", n.c_str());
			if (ImGui::Button("delete"))
			{
				DeleteLayout(n);
				s_listed = -10.0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
	ImGui::PopID();
}

bool LayoutCommand(const char* szLine)
{
	char verb[MAX_STRING] = { 0 };
	GetArg(verb, szLine, 3);
	std::string name = GetNextArg(szLine, 3);
	name.erase(std::remove(name.begin(), name.end(), '"'), name.end());

	if (!verb[0] || ci_equals(verb, "list"))
	{
		const std::vector<std::string> names = LayoutNames();
		WriteChatf("\ag[MQEQGFX]\ax layouts: %s", names.empty() ? "none" : "");
		for (const std::string& n : names)
			WriteChatf("\ag[MQEQGFX]\ax   %s", n.c_str());
		return true;
	}
	if (Clean(name).empty())
	{
		WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx ui layout <save|savebuttons|load|delete|list> <name>");
		return true;
	}
	if (ci_equals(verb, "save"))
		SaveLayout(name, false);
	else if (ci_equals(verb, "savebuttons"))
		SaveLayout(name, true);
	else if (ci_equals(verb, "load"))
		LoadLayout(name);
	else if (ci_equals(verb, "delete"))
	{
		if (!DeleteLayout(name))
			WriteChatf("\ag[MQEQGFX]\ax no layout named \ay%s\ax", name.c_str());
	}
	else
		WriteChatf("\ag[MQEQGFX]\ax usage: /eqgfx ui layout <save|savebuttons|load|delete|list> <name>");
	return true;
}

} // namespace eqgfx::ui
