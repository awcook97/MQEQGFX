#pragma once

#include "../SettingsFile.h"

#include <algorithm>
#include <string>
#include <string_view>

namespace eqgfx::ui {

void UpdateProfile();

void FlushProfile();

const std::string& ProfileClass();

int ProfileSerial();

void PlaceWindow(const char* id, bool withSize = false);
void NoteWindow(const char* id);

float ClassValue(const char* section, const char* key, float fallback);
void  SetClassValue(const char* section, const char* key, float value);
inline float ClassValue(const char* section, const char* key, float fallback, float lo, float hi)
{
	return std::clamp(ClassValue(section, key, fallback), lo, hi);
}
inline int  ClassInt(const char* section, const char* key, int fallback, int lo, int hi)
{
	return static_cast<int>(ClassValue(section, key, static_cast<float>(fallback), static_cast<float>(lo),
		static_cast<float>(hi)));
}
inline bool ClassBool(const char* section, const char* key, bool fallback)
{
	return ClassValue(section, key, fallback ? 1.0f : 0.0f) != 0.0f;
}

settings::json& ProfileRoot();
void MarkProfileDirty();

void ReloadProfile();

} // namespace eqgfx::ui
