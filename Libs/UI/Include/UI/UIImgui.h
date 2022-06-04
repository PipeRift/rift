// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Platform.h>
#include <Core/StringView.h>
#include <Math/Color.h>
#include <Math/Vector.h>


#define IM_VEC2_CLASS_EXTRA                                      \
	constexpr ImVec2(Pipe::v2 other) : x(other.x), y(other.y) {} \
	constexpr operator Pipe::v2() const                          \
	{                                                            \
		return Pipe::v2{x, y};                                   \
	}

#define IM_VEC4_CLASS_EXTRA                                                                     \
	constexpr ImVec4(const Pipe::LinearColor& other)                                            \
	    : x(other.r), y(other.g), z(other.b), w(other.a)                                        \
	{}                                                                                          \
	constexpr operator Pipe::LinearColor() const                                                \
	{                                                                                           \
		return Pipe::LinearColor{x, y, z, w};                                                   \
	}                                                                                           \
	constexpr ImVec4(const Pipe::v4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {} \
	constexpr operator Pipe::v4() const                                                         \
	{                                                                                           \
		return Pipe::v4{x, y, z, w};                                                            \
	}


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>


namespace Rift::UI
{
	using namespace Pipe;
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

	template<Integral T>
	inline void PushID(T id)
	{
		UI::PushID(reinterpret_cast<void*>(sizet(id)));
	}

	inline ImGuiID GetID(StringView id)
	{
		return UI::GetID(id.data(), id.data() + id.size());
	}

	inline void PushStyleColor(ImGuiCol idx, Color color)
	{
		ImGui::PushStyleColor(idx, color.DWColor());
	}

	inline void PushStyleColor(ImGuiCol idx, const LinearColor& color)
	{
		ImGui::PushStyleColor(idx, color);
	}

	inline void Text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextV(fmt, args);
		va_end(args);
	}
	inline void Text(StringView text)
	{
		TextUnformatted(text.data(), text.data() + text.size());
	}

	inline void TextColoredUnformatted(
	    const LinearColor& color, const char* text, const char* textEnd = nullptr)
	{
		PushStyleColor(ImGuiCol_Text, color);
		TextUnformatted(text, textEnd);
		PopStyleColor();
	}

	inline void TextColored(const LinearColor& color, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextColoredV(ImVec4(color), fmt, args);
		va_end(args);
	}
	inline void TextColored(const LinearColor& color, StringView text)
	{
		TextColoredUnformatted(color, text.data(), text.data() + text.size());
	}

}    // namespace Rift::UI
