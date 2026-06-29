// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <Pipe/Core/StringView.h>
#include <PipeColor.h>
#include <PipePlatform.h>
#include <PipeVectors.h>


#define IM_VEC2_CLASS_EXTRA                                   \
	constexpr ImVec2(p::v2 other) : x(other.x), y(other.y) {} \
	constexpr operator p::v2() const                          \
	{                                                         \
		return p::v2{x, y};                                   \
	}

#define IM_VEC4_CLASS_EXTRA                                                                        \
	constexpr ImVec4(const p::LinearColor& other) : x(other.r), y(other.g), z(other.b), w(other.a) \
	{}                                                                                             \
	constexpr operator p::LinearColor() const                                                      \
	{                                                                                              \
		return p::LinearColor{x, y, z, w};                                                         \
	}                                                                                              \
	constexpr ImVec4(const p::v4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}       \
	constexpr operator p::v4() const                                                               \
	{                                                                                              \
		return p::v4{x, y, z, w};                                                                  \
	}


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <Misc/PipeImGui.h>


namespace rift::UI
{
	using namespace ImGui;
}    // namespace rift::UI
