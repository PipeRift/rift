// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Paths.h"
#include "UI/Style.h"
#include "UI/UI.h"

#include <Containers/Array.h>
#include <Log.h>
#include <Math/Color.h>
#include <Math/Math.h>
#include <Templates/Tuples.h>


using namespace Rift;

namespace Style
{
	struct FontType
	{
		TArray<TPair<float, ImFont*>> sizes{};

		void Add(float size, ImFont* imFont)
		{
			if (sizes.Contains([size](const auto& font) {
				    return Math::NearlyEqual(font.first, size);
			    }))
			{
				Log::Error(
				    "Tried to register the same font with the same size and mode twice (size: {})",
				    size);
				return;
			}
			sizes.Add({size, imFont});
		}

		ImFont* Get(float desiredSize = 0.f) const
		{
			if (sizes.IsEmpty())
			{
				return nullptr;
			}
			if (desiredSize <= 0.f)
			{
				return sizes.First().second;
			}
			const TPair<float, ImFont*>* foundFont = sizes.Find([desiredSize](const auto& font) {
				return Math::NearlyEqual(font.first, desiredSize);
			});
			return foundFont ? foundFont->second : nullptr;
		}
	};

	struct FontDescriptor
	{
		std::array<FontType, Rift::Refl::GetEnumSize<FontMode>()> modes{};

		FontType& operator[](FontMode mode)
		{
			return modes[u8(mode)];
		}
		const FontType& operator[](FontMode mode) const
		{
			return modes[u8(mode)];
		}
	};

	static TMap<Rift::Name, FontDescriptor> fonts{};


	void AddFont(Name name, FontMode mode, float size, Path file)
	{
		FontDescriptor* font = fonts.Find(name);
		if (!font)
		{
			fonts.Insert(name, {});
			font = &fonts[name];
		}

		auto& io = ImGui::GetIO();
		(*font)[mode].Add(size, io.Fonts->AddFontFromFileTTF(Paths::ToString(file).data(), size));
	}

	void LoadFonts()
	{
		auto& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();

		auto resources = Paths::GetResourcesPath() / "Editor";

		// Work Sans
		AddFont("WorkSans", FontMode::Bold, 14.f, resources / "Fonts/WorkSans-Bold.ttf");
		AddFont(
		    "WorkSans", FontMode::BoldItalic, 14.f, resources / "Fonts/WorkSans-BoldItalic.ttf");
		AddFont("WorkSans", FontMode::Italic, 14.f, resources / "Fonts/WorkSans-Italic.ttf");
		AddFont("WorkSans", FontMode::Light, 14.f, resources / "Fonts/WorkSans-Light.ttf");
		AddFont(
		    "WorkSans", FontMode::LightItalic, 14.f, resources / "Fonts/WorkSans-LightItalic.ttf");
		AddFont("WorkSans", FontMode::Regular, 14.f, resources / "Fonts/WorkSans-Regular.ttf");

		// Karla
		AddFont("Karla", FontMode::Bold, 14.f, resources / "Fonts/Karla-Bold.ttf");
		AddFont("Karla", FontMode::BoldItalic, 14.f, resources / "Fonts/Karla-BoldItalic.ttf");
		AddFont("Karla", FontMode::Italic, 14.f, resources / "Fonts/Karla-Italic.ttf");
		AddFont("Karla", FontMode::Light, 14.f, resources / "Fonts/Karla-Light.ttf");
		AddFont("Karla", FontMode::LightItalic, 14.f, resources / "Fonts/Karla-LightItalic.ttf");
		AddFont("Karla", FontMode::Regular, 14.f, resources / "Fonts/Karla-Regular.ttf");
	}

	ImFont* FindFont(Rift::Name name, FontMode mode, float size)
	{
		const FontDescriptor* const font = fonts.Find(name);
		return font ? (*font)[mode].Get(size) : nullptr;
	}

	void SetDefaultFont(Rift::Name name, FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			Log::Error("Tried to set inexistent font '{}' as default", name);
		}
		ImGui::GetIO().FontDefault = font;
	}

	void PushFont(Rift::Name name, FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			Log::Error("Tried to push inexistent font '{}' (mode: {}, size: {})", name,
			    Refl::GetEnumName(mode), size);
		}
		ImGui::PushFont(font);
	}

	void PopFont()
	{
		ImGui::PopFont();
	}

	void ApplyStyle()
	{
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();

		ImVec4* colors                      = style.Colors;
		colors[ImGuiCol_FrameBg]            = LinearColor(0.48f, 0.40f, 0.16f, 0.54f);
		colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
		colors[ImGuiCol_FrameBgActive]      = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
		colors[ImGuiCol_TitleBgActive]      = ImVec4(0.48f, 0.40f, 0.16f, 1.00f);
		colors[ImGuiCol_CheckMark]          = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_SliderGrab]         = ImVec4(0.88f, 0.69f, 0.24f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_Button]             = ImVec4(0.98f, 0.77f, 0.26f, 0.40f);
		colors[ImGuiCol_ButtonHovered]      = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_ButtonActive]       = ImVec4(0.98f, 0.65f, 0.06f, 1.00f);
		colors[ImGuiCol_Header]             = ImVec4(0.98f, 0.77f, 0.26f, 0.31f);
		colors[ImGuiCol_HeaderHovered]      = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
		colors[ImGuiCol_HeaderActive]       = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);
		colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.75f, 0.54f, 0.10f, 0.78f);
		colors[ImGuiCol_SeparatorActive]    = ImVec4(0.75f, 0.54f, 0.10f, 1.00f);
		colors[ImGuiCol_ResizeGrip]         = ImVec4(0.98f, 0.77f, 0.26f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.98f, 0.77f, 0.26f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.98f, 0.77f, 0.26f, 0.95f);
		colors[ImGuiCol_Tab]                = ImVec4(0.58f, 0.47f, 0.18f, 0.86f);
		colors[ImGuiCol_TabHovered]         = ImVec4(0.98f, 0.77f, 0.26f, 0.80f);
		colors[ImGuiCol_TabActive]          = ImVec4(0.68f, 0.54f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.13f, 0.07f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.34f, 0.14f, 1.00f);
		colors[ImGuiCol_DockingPreview]     = ImVec4(0.98f, 0.77f, 0.26f, 0.70f);
		colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.98f, 0.77f, 0.26f, 0.35f);
		colors[ImGuiCol_NavHighlight]       = ImVec4(0.98f, 0.77f, 0.26f, 1.00f);

		style.WindowRounding    = 2;
		style.TabRounding       = 1;
		style.ScrollbarRounding = 2;

		LoadFonts();
		Style::SetDefaultFont("WorkSans");
	}
}    // namespace Style
