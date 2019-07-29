// imgui_stl.cpp
// Wrappers for C++ standard library (STL) types (eastl::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Compatibility:
// - eastl::string support is only guaranteed to work from C++11. 
//   If you try to use it pre-C++11, please share your findings (w/ info about compiler/architecture)

#include "imgui.h"
#include "imgui_stl.h"

void ImGui::Text(String&& txt)
{
	ImGui::TextUnformatted(txt.cbegin(), txt.cend());
}

void ImGui::Text(const String& txt)
{
	ImGui::TextUnformatted(txt.cbegin(), txt.cend());
}

struct InputTextCallback_UserData
{
	String*             Str;
	ImGuiInputTextCallback  ChainCallback;
	void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		String* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback)
	{
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}

bool ImGui::InputText(const char* label, String& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = &str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;
	return InputText(label, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, String& str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = &str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;
	return InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}
