#pragma once

#include "nlohmann/json.hpp"

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace eqgfx::settings {

using json = nlohmann::json;

using IniKeys = std::vector<std::pair<std::string, std::string>>;
using IniFile = std::vector<std::pair<std::string, IniKeys>>;
IniFile ReadIni(const std::filesystem::path& path);

json FromIni(const IniFile& ini);
json FromIniValue(const std::string& value);

class Section
{
public:
	void Put(const char* key, const char* value) { m_obj[key] = value ? value : ""; }
	void Put(const char* key, const std::string& value) { m_obj[key] = value; }
	void PutBool(const char* key, bool value)    { m_obj[key] = value; }
	void PutInt(const char* key, int value)      { m_obj[key] = value; }
	void PutFloat(const char* key, float value)  { m_obj[key] = value; }

	json Take() { return std::move(m_obj); }

private:
	json m_obj = json::object();
};

class File
{
public:
	using Importer = std::function<json(const IniFile&)>;

	void Open(std::filesystem::path path, const std::filesystem::path& legacyIni = {}, Importer importer = {});
	void Close();
	bool IsOpen() const { return m_open; }
	const std::filesystem::path& Path() const { return m_path; }

	json& Root() { return m_root; }
	const json& Root() const { return m_root; }

	const json* Find(std::string_view section, std::string_view key) const;
	bool        GetBool(std::string_view section, std::string_view key, bool fallback) const;
	int         GetInt(std::string_view section, std::string_view key, int fallback) const;
	float       GetFloat(std::string_view section, std::string_view key, float fallback) const;
	std::string GetString(std::string_view section, std::string_view key, std::string_view fallback) const;

	void Set(std::string_view section, std::string_view key, json value);
	void SetSection(std::string_view section, json value);
	void SetSection(std::string_view section, Section&& built) { SetSection(section, built.Take()); }

	void MarkDirty();
	void Pulse();
	void Save();

private:
	std::filesystem::path m_path;
	json                  m_root = json::object();
	json                  m_base = json::object();
	bool                  m_open = false;
	bool                  m_dirty = false;
	std::chrono::steady_clock::time_point m_changedAt;
};

File& Global();

std::filesystem::path Dir();
std::filesystem::path ClassesDir();
std::filesystem::path LooksDir();
std::filesystem::path LayoutsDir();
std::filesystem::path LuaDir();
void PrepareConfigDirs();

json ReadJson(const std::filesystem::path& path);
void WriteJson(const std::filesystem::path& path, const json& doc);

void FlushWrites();

bool        AsBool(const json& v, bool fallback);
int         AsInt(const json& v, int fallback);
float       AsFloat(const json& v, float fallback);
std::string AsString(const json& v, std::string_view fallback);

} // namespace eqgfx::settings
