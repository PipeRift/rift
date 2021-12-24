// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Math/Color.h>
#include <Math/Vector.h>
#include <Platform/Platform.h>
#include <Strings/StringView.h>


#define IM_VEC2_CLASS_EXTRA                                      \
	constexpr ImVec2(Rift::v2 other) : x(other.x), y(other.y) {} \
	constexpr operator Rift::v2() const                          \
	{                                                            \
		return Rift::v2{x, y};                                   \
	}

#define IM_VEC4_CLASS_EXTRA                                                                     \
	constexpr ImVec4(const Rift::LinearColor& other)                                            \
	    : x(other.r), y(other.g), z(other.b), w(other.a)                                        \
	{}                                                                                          \
	constexpr operator Rift::LinearColor() const                                                \
	{                                                                                           \
		return Rift::LinearColor{x, y, z, w};                                                   \
	}                                                                                           \
	constexpr ImVec4(const Rift::v4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {} \
	constexpr operator Rift::v4() const                                                         \
	{                                                                                           \
		return Rift::v4{x, y, z, w};                                                            \
	}


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>


namespace Rift::UI
{
	using namespace ImGui;

	// Begin ImGui API override
	// Push string into the ID stack (will hash string)
	inline void PushID(const char* str_id)
	{
		ImGui::PushID(str_id);
	}
	// Push string into the ID stack (will hash string)
	inline void PushID(const char* str_id_begin, const char* str_id_end)
	{
		ImGui::PushID(str_id_begin, str_id_end);
	}
	// Push pointer into the ID stack (will hash pointer)
	inline void PushID(const void* ptr_id)
	{
		ImGui::PushID(ptr_id);
	}
	// Push integer into the ID stack (will hash integer)
	inline void PushID(i32 int_id)
	{
		ImGui::PushID(int_id);
	}

	inline ImGuiID GetID(const char* str_id)
	{
		return ImGui::GetID(str_id);
	}
	inline ImGuiID GetID(const char* str_id_begin, const char* str_id_end)
	{
		return ImGui::GetID(str_id_begin, str_id_end);
	}
	inline ImGuiID GetID(const void* ptr_id)
	{
		return ImGui::GetID(ptr_id);
	}
	inline void PushStyleColor(ImGuiCol idx, u32 col)
	{
		ImGui::PushStyleColor(idx, col);
	}
	inline void PushStyleColor(ImGuiCol idx, const ImVec4& col)
	{
		ImGui::PushStyleColor(idx, col);
	}
	// End ImGui API override


	inline void PushID(StringView id)
	{
		UI::PushID(id.data(), id.data() + id.size());
	}

	inline void PushID(sizet sizet_id)
	{
		UI::PushID(reinterpret_cast<void*>(sizet_id));
	}

	inline ImGuiID GetID(StringView id)
	{
		return UI::GetID(id.data(), id.data() + id.size());
	}

	inline void PushStyleColor(ImGuiCol idx, const LinearColor& color)
	{
		UI::PushStyleColor(idx, ImVec4{color.r, color.g, color.b, color.a});
	}

	inline void PushStyleColor(ImGuiCol idx, Color color)
	{
		UI::PushStyleColor(idx, color.DWColor());
	}
}    // namespace Rift::UI
