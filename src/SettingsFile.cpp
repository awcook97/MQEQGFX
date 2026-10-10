#include "SettingsFile.h"

#include <mq/Plugin.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <system_error>
#include <thread>

namespace eqgfx::settings {

namespace {

constexpr auto kIdle = std::chrono::milliseconds(1500);

std::mutex              s_mutex;
std::condition_variable s_wake;
std::condition_variable s_idle;
struct Pending
{
	json ops = json::array();
	json fallback;
};
std::map<std::filesystem::path, Pending> s_pending;
std::thread s_worker;
bool        s_busy = false;
bool        s_stop = false;
void ApplyOps(json& doc, const json& ops)
{
	for (const json& op : ops)
	{
		if (op.contains("all"))
		{
			doc = op["all"];
			continue;
		}
		if (!doc.is_object())
			doc = json::object();
		const std::string& top = op["t"].get_ref<const std::string&>();
		if (op.contains("k"))
		{
			json& sec = doc[top];
			if (!sec.is_object())
				sec = json::object();
			const std::string& key = op["k"].get_ref<const std::string&>();
			if (op.contains("v"))
				sec[key] = op["v"];
			else
				sec.erase(key);
		}
		else if (op.contains("v"))
			doc[top] = op["v"];
		else
			doc.erase(top);
	}
}

void WriteNow(const std::filesystem::path& path, const Pending& item)
{
	json doc = ReadJson(path);
	if (!doc.is_object())
		doc = item.fallback.is_object() ? item.fallback : json::object();
	ApplyOps(doc, item.ops);

	std::error_code ec;
	std::filesystem::create_directories(path.parent_path(), ec);
	std::filesystem::path temp = path;
	temp += "." + std::to_string(GetCurrentProcessId()) + ".tmp";
	bool written = false;
	{
		std::ofstream out(temp, std::ios::binary | std::ios::trunc);
		out << doc.dump(1, '\t', false, json::error_handler_t::replace);
		written = static_cast<bool>(out);
	}
	if (written)
		std::filesystem::rename(temp, path, ec);
	if (!written || ec)
		std::filesystem::remove(temp, ec);
}

void Worker()
{
	std::unique_lock lock(s_mutex);
	for (;;)
	{
		s_wake.wait(lock, [] { return s_stop || !s_pending.empty(); });
		if (s_pending.empty())
			return;
		auto node = s_pending.extract(s_pending.begin());
		s_busy = true;
		lock.unlock();

		WriteNow(node.key(), node.mapped());

		lock.lock();
		s_busy = false;
		if (s_pending.empty())
			s_idle.notify_all();
	}
}

void Queue(const std::filesystem::path& path, json ops, json fallback)
{
	std::scoped_lock lock(s_mutex);
	Pending& p = s_pending[path];
	for (json& op : ops)
		p.ops.push_back(std::move(op));
	p.fallback = std::move(fallback);
	if (!s_worker.joinable())
	{
		s_stop = false;
		s_worker = std::thread(Worker);
	}
	s_wake.notify_one();
}

std::vector<std::string> KeysOf(const json& a, const json& b)
{
	std::vector<std::string> keys;
	for (const json* o : { &a, &b })
		if (o->is_object())
			for (auto it = o->begin(); it != o->end(); ++it)
				keys.push_back(it.key());
	std::sort(keys.begin(), keys.end());
	keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
	return keys;
}

const json* Member(const json& obj, const std::string& key)
{
	if (!obj.is_object())
		return nullptr;
	auto it = obj.find(key);
	return it != obj.end() ? &*it : nullptr;
}

json Diff(const json& base, const json& root)
{
	json ops = json::array();
	for (const std::string& top : KeysOf(base, root))
	{
		const json* b = Member(base, top);
		const json* r = Member(root, top);
		if (b && r && *b == *r)
			continue;
		if (b && r && b->is_object() && r->is_object())
		{
			for (const std::string& key : KeysOf(*b, *r))
			{
				const json* bv = Member(*b, key);
				const json* rv = Member(*r, key);
				if (bv && rv && *bv == *rv)
					continue;
				json op = { { "t", top }, { "k", key } };
				if (rv)
					op["v"] = *rv;
				ops.push_back(std::move(op));
			}
			continue;
		}
		json op = { { "t", top } };
		if (r)
			op["v"] = *r;
		ops.push_back(std::move(op));
	}
	return ops;
}

bool ParseNumber(const std::string& text, json& out)
{
	if (text.empty() || std::isspace(static_cast<unsigned char>(text.front())))
		return false;
	char* end = nullptr;
	errno = 0;
	const long long whole = std::strtoll(text.c_str(), &end, 10);
	if (end && *end == 0 && errno == 0)
	{
		out = whole;
		return true;
	}
	errno = 0;
	const double real = std::strtod(text.c_str(), &end);
	if (end && *end == 0 && errno == 0 && std::isfinite(real))
	{
		out = real;
		return true;
	}
	return false;
}

} // namespace

IniFile ReadIni(const std::filesystem::path& path)
{
	IniFile out;
	std::ifstream in(path);
	std::string line;
	IniKeys* current = nullptr;
	while (std::getline(in, line))
	{
		while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
			line.pop_back();
		if (line.empty() || line[0] == ';')
			continue;
		if (line[0] == '[')
		{
			const size_t close = line.find(']');
			current = nullptr;
			if (close != std::string::npos)
			{
				out.emplace_back(line.substr(1, close - 1), IniKeys{});
				current = &out.back().second;
			}
			continue;
		}
		const size_t eq = line.find('=');
		if (current && eq != std::string::npos)
			current->emplace_back(line.substr(0, eq), line.substr(eq + 1));
	}
	return out;
}

json FromIniValue(const std::string& value)
{
	json number;
	return ParseNumber(value, number) ? number : json(value);
}

json FromIni(const IniFile& ini)
{
	json root = json::object();
	for (const auto& [section, keys] : ini)
	{
		json& obj = root[section];
		if (!obj.is_object())
			obj = json::object();
		for (const auto& [key, value] : keys)
			obj[key] = FromIniValue(value);
	}
	return root;
}

bool AsBool(const json& v, bool fallback)
{
	if (v.is_boolean())
		return v.get<bool>();
	if (v.is_number())
		return v.get<double>() != 0.0;
	if (v.is_string())
	{
		const std::string& s = v.get_ref<const std::string&>();
		if (s == "1" || s == "true" || s == "True" || s == "on")
			return true;
		if (s == "0" || s == "false" || s == "False" || s == "off")
			return false;
	}
	return fallback;
}

int AsInt(const json& v, int fallback)
{
	if (v.is_number_unsigned())
		return static_cast<int>(std::min<unsigned long long>(v.get<unsigned long long>(), INT_MAX));
	if (v.is_number_integer())
		return static_cast<int>(std::clamp<long long>(v.get<long long>(), INT_MIN, INT_MAX));
	if (v.is_number())
	{
		const double d = v.get<double>();
		if (!std::isfinite(d))
			return fallback;
		return static_cast<int>(std::clamp<double>(d, INT_MIN, INT_MAX));
	}
	if (v.is_boolean())
		return v.get<bool>() ? 1 : 0;
	if (v.is_string())
	{
		json n;
		if (ParseNumber(v.get_ref<const std::string&>(), n))
			return AsInt(n, fallback);
	}
	return fallback;
}

float AsFloat(const json& v, float fallback)
{
	if (v.is_number())
	{
		const double d = v.get<double>();
		if (!std::isfinite(d))
			return fallback;
		return static_cast<float>(std::clamp<double>(d, -FLT_MAX, FLT_MAX));
	}
	if (v.is_boolean())
		return v.get<bool>() ? 1.0f : 0.0f;
	if (v.is_string())
	{
		json n;
		if (ParseNumber(v.get_ref<const std::string&>(), n))
			return AsFloat(n, fallback);
	}
	return fallback;
}

std::string AsString(const json& v, std::string_view fallback)
{
	if (v.is_string())
		return v.get<std::string>();
	if (v.is_boolean())
		return v.get<bool>() ? "1" : "0";
	if (v.is_number_integer())
		return std::to_string(v.get<long long>());
	if (v.is_number())
	{
		char text[32];
		std::snprintf(text, sizeof(text), "%g", v.get<double>());
		return text;
	}
	return std::string(fallback);
}

void File::Open(std::filesystem::path path, const std::filesystem::path& legacyIni, Importer importer)
{
	Close();
	m_path = std::move(path);
	m_root = json::object();
	m_base = json::object();
	m_open = true;

	std::error_code ec;
	if (std::filesystem::exists(m_path, ec))
	{
		json parsed = ReadJson(m_path);
		if (parsed.is_object())
		{
			m_root = std::move(parsed);
			m_base = m_root;
		}
		else
		{
			WriteChatf("\ag[MQEQGFX]\ax %s is not valid JSON, using defaults", m_path.filename().string().c_str());
		}
		return;
	}
	if (!legacyIni.empty() && std::filesystem::exists(legacyIni, ec))
	{
		const IniFile ini = ReadIni(legacyIni);
		m_root = importer ? importer(ini) : FromIni(ini);
		if (!m_root.is_object())
			m_root = json::object();
		Save();
	}
}

void File::Close()
{
	if (m_open && m_dirty)
		Save();
	m_open = false;
	m_dirty = false;
	m_root = json::object();
	m_base = json::object();
}

const json* File::Find(std::string_view section, std::string_view key) const
{
	auto sec = m_root.find(section);
	if (sec == m_root.end() || !sec->is_object())
		return nullptr;
	auto it = sec->find(key);
	return it != sec->end() ? &*it : nullptr;
}

bool File::GetBool(std::string_view section, std::string_view key, bool fallback) const
{
	const json* v = Find(section, key);
	return v ? AsBool(*v, fallback) : fallback;
}

int File::GetInt(std::string_view section, std::string_view key, int fallback) const
{
	const json* v = Find(section, key);
	return v ? AsInt(*v, fallback) : fallback;
}

float File::GetFloat(std::string_view section, std::string_view key, float fallback) const
{
	const json* v = Find(section, key);
	return v ? AsFloat(*v, fallback) : fallback;
}

std::string File::GetString(std::string_view section, std::string_view key, std::string_view fallback) const
{
	const json* v = Find(section, key);
	return v ? AsString(*v, fallback) : std::string(fallback);
}

void File::Set(std::string_view section, std::string_view key, json value)
{
	json& sec = m_root[std::string(section)];
	if (!sec.is_object())
		sec = json::object();
	sec[std::string(key)] = std::move(value);
	MarkDirty();
}

void File::SetSection(std::string_view section, json value)
{
	m_root[std::string(section)] = std::move(value);
	MarkDirty();
}

void File::MarkDirty()
{
	if (!m_open)
		return;
	m_dirty = true;
	m_changedAt = std::chrono::steady_clock::now();
}

void File::Pulse()
{
	if (m_open && m_dirty && std::chrono::steady_clock::now() - m_changedAt >= kIdle)
		Save();
}

void File::Save()
{
	if (!m_open || m_path.empty())
		return;
	m_dirty = false;
	json ops = Diff(m_base, m_root);
	if (ops.empty())
		return;
	m_base = m_root;
	Queue(m_path, std::move(ops), m_root);
}

json ReadJson(const std::filesystem::path& path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return nullptr;
	json parsed = json::parse(in, nullptr, false);
	return parsed.is_discarded() ? json(nullptr) : parsed;
}

void WriteJson(const std::filesystem::path& path, const json& doc)
{
	json ops = json::array();
	ops.push_back({ { "all", doc } });
	Queue(path, std::move(ops), doc);
}

File& Global()
{
	static File file;
	return file;
}

void FlushWrites()
{
	{
		std::unique_lock lock(s_mutex);
		if (!s_worker.joinable())
			return;
		s_idle.wait(lock, [] { return s_pending.empty() && !s_busy; });
		s_stop = true;
	}
	s_wake.notify_one();
	s_worker.join();
}

} // namespace eqgfx::settings
