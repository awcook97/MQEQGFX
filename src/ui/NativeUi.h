#pragma once

namespace eqgfx::ui {

void InitNativeUi();
void ShutdownNativeUi();
void PulseNativeUi();
void DrawNativeUi();
void NativeUiCleanUI();
void NativeUiReloadUI();
bool NativeUiCommand(const char* szLine);
void NativeUiIncomingChat(const char* line, unsigned long color);
void NativeUiWriteChat(const char* line, int color);
void DrawNativeUiOptions();
void DrawNativeUiDebug();

} // namespace eqgfx::ui
