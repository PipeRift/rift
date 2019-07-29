// imgui_stl.h
// Wrappers for C++ standard library (STL) types (eastl::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Compatibility:
// - eastl::string support is only guaranteed to work from C++11.
//   If you try to use it pre-C++11, please share your findings (w/ info about compiler/architecture)

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with eastl::string

#pragma once

#include "Core/Strings/String.h"
#include "imgui.h"

namespace ImGui
{
	IMGUI_API void Text(String&& txt);
	IMGUI_API void Text(const String& txt);


	// ImGui::InputText() with eastl::string
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	IMGUI_API bool  InputText(const char* label, String& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	IMGUI_API bool  InputTextMultiline(const char* label, String& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}
