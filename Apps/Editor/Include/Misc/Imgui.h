// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Math/Color.h>
#include <Math/Vector.h>


#define IM_VEC2_CLASS_EXTRA                                      \
	constexpr ImVec2(Rift::v2 other) : x(other.x), y(other.y) {} \
	constexpr ImVec2& operator=(Rift::v2 other)                  \
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
