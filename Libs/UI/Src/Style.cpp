// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Paths.h"
#include "UI/Style.h"
#include "UI/UIImGui.h"

#include <Containers/Array.h>
#include <Log.h>
#include <Math/Math.h>
#include <Misc/Checks.h>
#include <Templates/Tuples.h>


using namespace Rift;

namespace Rift::Style
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

	LinearColor Hovered(const LinearColor& color)
	{
		return color.Darken(0.1f);
	}

	LinearColor Disabled(const LinearColor& color)
	{
		return color.Darken(0.3f);
	}

	void SetTheme(ThemeColors themeColors)
	{
		LinearColor mainColor;
		LinearColor mainTextColor;
		switch (themeColors)
		{
			case ThemeColors::Primary:
				mainColor     = primaryColor;
				mainTextColor = primaryTextColor;
				break;
			case ThemeColors::Secondary:
				mainColor     = secondaryColor;
				mainTextColor = secondaryTextColor;
				break;
			default:
				mainColor     = fillColor;
				mainTextColor = secondaryTextColor;
				break;
		}
	}

	void PushGeneralStyle()
	{
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();

		style.WindowRounding           = 0;
		style.FramePadding             = ImVec2(2.f, 4.5f);
		style.TabRounding              = 0;
		style.ScrollbarRounding        = 2;
		style.WindowMenuButtonPosition = ImGuiDir_Right;

		// PushThemeStyle(ThemeColors::Primary);


		ImVec4* colors                  = style.Colors;
		LinearColor frameColor          = fillColor.Darken(0.1f);
		colors[ImGuiCol_FrameBg]        = frameColor.Darken(0.5f);
		colors[ImGuiCol_FrameBgHovered] = frameColor.Darken(0.1f);
		colors[ImGuiCol_FrameBgActive]  = frameColor;

		LinearColor titleColor            = primaryColor.Darken(0.5f);
		colors[ImGuiCol_TitleBg]          = titleColor.Darken(0.65f);
		colors[ImGuiCol_TitleBgActive]    = titleColor.Darken(0.3f);
		colors[ImGuiCol_TitleBgCollapsed] = Disabled(titleColor);

		colors[ImGuiCol_CheckMark]        = fillTextColor.Darken(0.2f);
		colors[ImGuiCol_SliderGrabActive] = fillColor;
		colors[ImGuiCol_SliderGrab]       = fillColor.Darken(0.2f);

		LinearColor buttonColor        = fillColor;
		colors[ImGuiCol_Button]        = buttonColor.Darken(0.3f);
		colors[ImGuiCol_ButtonHovered] = Hovered(buttonColor);
		colors[ImGuiCol_ButtonActive]  = buttonColor;

		LinearColor headerColor        = fillColor;
		colors[ImGuiCol_Header]        = headerColor.Darken(0.3f);
		colors[ImGuiCol_HeaderHovered] = Hovered(headerColor);
		colors[ImGuiCol_HeaderActive]  = headerColor;

		LinearColor separatorColor        = fillColor;
		colors[ImGuiCol_SeparatorHovered] = Hovered(separatorColor);
		colors[ImGuiCol_SeparatorActive]  = separatorColor;

		LinearColor resizeGripColor        = fillColor;
		colors[ImGuiCol_ResizeGrip]        = resizeGripColor.Darken(0.3f);
		colors[ImGuiCol_ResizeGripHovered] = Hovered(resizeGripColor);
		colors[ImGuiCol_ResizeGripActive]  = resizeGripColor;

		LinearColor tabColor                = primaryColor.Darken(0.1f);
		colors[ImGuiCol_Tab]                = tabColor.Darken(0.3f);
		colors[ImGuiCol_TabActive]          = tabColor;
		colors[ImGuiCol_TabUnfocused]       = tabColor.Darken(0.5f);
		colors[ImGuiCol_TabUnfocusedActive] = tabColor.Darken(0.3f);
		colors[ImGuiCol_TabHovered]         = Hovered(tabColor);

		colors[ImGuiCol_DockingPreview] = fillColor;
		colors[ImGuiCol_DockingEmptyBg] = LinearColor(Color::White).Darken(0.97f);
		colors[ImGuiCol_TextSelectedBg] = primaryColor.Darken(0.2f);

		colors[ImGuiCol_NavHighlight] = primaryColor;

		colors[ImGuiCol_Border] = fillColor.Darken(0.1f).Translucency(0.5f);

		colors[ImGuiCol_Text]         = fillTextColor;
		colors[ImGuiCol_TextDisabled] = fillTextColor.Darken(0.1f);

		LoadFonts();
		Style::SetDefaultFont("WorkSans");
	}

	void PopGeneralStyle()
	{
		// PopThemeStyle();
	}

	// Make the UI compact because there are so many fields
	void PushStyleCompact()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
		    ImVec2(style.FramePadding.x, (float) (int) (style.FramePadding.y * 0.60f)));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
		    ImVec2(style.ItemSpacing.x, (float) (int) (style.ItemSpacing.y * 0.60f)));
	}

	void PopStyleCompact()
	{
		ImGui::PopStyleVar(2);
	}
}    // namespace Rift::Style
