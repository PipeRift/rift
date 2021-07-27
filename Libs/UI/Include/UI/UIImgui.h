// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Math/Color.h>
#include <Math/Vector.h>


#define IM_VEC2_CLASS_EXTRA                                      \
	constexpr ImVec2(Rift::v2 other) : x(other.x), y(other.y) {} \
	constexpr ImVec2& operator=(Rift::Vec<2, float> other)       \
	{                                                            \
		x = other.x;                                             \
		y = other.y;                                             \
		return *this;                                            \
	}                                                            \
	constexpr ImVec2& operator+=(ImVec2 other)                   \
	{                                                            \
		x += other.x;                                            \
		y += other.y;                                            \
		return *this;                                            \
	}                                                            \
	constexpr ImVec2 operator+(ImVec2 other) const               \
	{                                                            \
		ImVec2 newVector = *this;                                \
		newVector += other;                                      \
		return newVector;                                        \
	}                                                            \
	constexpr ImVec2& operator-=(ImVec2 other)                   \
	{                                                            \
		x -= other.x;                                            \
		y -= other.y;                                            \
		return *this;                                            \
	}                                                            \
	constexpr ImVec2 operator-(ImVec2 other) const               \
	{                                                            \
		ImVec2 newVector = *this;                                \
		newVector -= other;                                      \
		return newVector;                                        \
	}

#define IM_VEC4_CLASS_EXTRA                                                                       \
	constexpr ImVec4(Rift::LinearColor other) : x(other.r), y(other.g), z(other.b), w(other.a) {} \
	constexpr ImVec4& operator=(const Rift::LinearColor& other)                                   \
	{                                                                                             \
		x = other.r;                                                                              \
		y = other.g;                                                                              \
		z = other.b;                                                                              \
		w = other.a;                                                                              \
		return *this;                                                                             \
	}                                                                                             \
	constexpr ImVec4& operator+=(const Rift::LinearColor& other)                                  \
	{                                                                                             \
		x += other.r;                                                                             \
		y += other.g;                                                                             \
		z += other.b;                                                                             \
		w += other.a;                                                                             \
		return *this;                                                                             \
	}                                                                                             \
	constexpr ImVec4 operator+(const Rift::LinearColor& other) const                              \
	{                                                                                             \
		ImVec4 newVector = *this;                                                                 \
		newVector += other;                                                                       \
		return newVector;                                                                         \
	}                                                                                             \
	constexpr ImVec4& operator-=(const Rift::LinearColor& other)                                  \
	{                                                                                             \
		x -= other.r;                                                                             \
		y -= other.g;                                                                             \
		z -= other.b;                                                                             \
		w -= other.a;                                                                             \
		return *this;                                                                             \
	}                                                                                             \
	constexpr ImVec4 operator-(const Rift::LinearColor& other) const                              \
	{                                                                                             \
		ImVec4 newVector = *this;                                                                 \
		newVector -= other;                                                                       \
		return newVector;                                                                         \
	}


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
	// End ImGui API override


	inline void PushID(StringView id)
	{
		UI::PushID(id.data(), id.data() + id.size());
	}

	inline ImGuiID GetID(StringView id)
	{
		return UI::GetID(id.data(), id.data() + id.size());
	}
}    // namespace Rift::UI
