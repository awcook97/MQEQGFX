#pragma once

#include <string>
#include <vector>

namespace eqgfx::ui {

std::vector<std::string> LayoutNames();
bool SaveLayout(const std::string& name, bool withButtons);
bool LoadLayout(const std::string& name);
bool DeleteLayout(const std::string& name);

void DrawLayoutOptions();
bool LayoutCommand(const char* szLine);

} // namespace eqgfx::ui
