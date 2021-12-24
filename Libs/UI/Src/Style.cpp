// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Style.h"

#include "UI/Paths.h"
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

	static TMap<Rift::Name, FontDescriptor> gFonts{};


	void AddFont(Name name, FontMode mode, float size, Path file)
	{
		FontDescriptor* font = gFonts.Find(name);
		if (!font)
		{
			gFonts.Insert(name, {});
			font = &gFonts[name];
		}

		auto& io       = ImGui::GetIO();
		ImFont* imFont = io.Fonts->AddFontFromFileTTF(Paths::ToString(file).data(), size);
		(*font)[mode].Add(size, imFont);
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
		const FontDescriptor* const font = gFonts.Find(name);
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

	void PushGeneralStyle()
	{
		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();

		style.WindowRounding           = 1;
		style.FramePadding             = ImVec2(2.f, 4.5f);
		style.TabRounding              = 0;
		style.ScrollbarRounding        = 2;
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.PopupBorderSize          = 0.f;


		ImVec4* colors = style.Colors;

		LinearColor titleColor            = GetNeutralColor(0);
		colors[ImGuiCol_TitleBg]          = titleColor.Shade(0.2f);
		colors[ImGuiCol_TitleBgActive]    = titleColor;
		colors[ImGuiCol_TitleBgCollapsed] = Disabled(titleColor);

		colors[ImGuiCol_WindowBg] = GetNeutralColor(1);
		colors[ImGuiCol_Border]   = GetNeutralColor(0);

		colors[ImGuiCol_CheckMark]        = whiteTextColor;
		colors[ImGuiCol_SliderGrabActive] = GetNeutralColor(5);
		colors[ImGuiCol_SliderGrab]       = GetNeutralColor(4);


		LinearColor separatorColor        = GetNeutralColor(1);
		colors[ImGuiCol_SeparatorHovered] = Hovered(separatorColor);
		colors[ImGuiCol_SeparatorActive]  = separatorColor;

		LinearColor resizeGripColor        = GetNeutralColor(1);
		colors[ImGuiCol_ResizeGrip]        = resizeGripColor.Shade(0.3f);
		colors[ImGuiCol_ResizeGripHovered] = Hovered(resizeGripColor);
		colors[ImGuiCol_ResizeGripActive]  = resizeGripColor;

		colors[ImGuiCol_DockingPreview] = GetNeutralColor(2);
		colors[ImGuiCol_DockingEmptyBg] = LinearColor::White().Shade(0.97f);
		colors[ImGuiCol_TextSelectedBg] = primaryColor.Shade(0.1f);

		colors[ImGuiCol_NavHighlight] = primaryColor;

		// colors[ImGuiCol_Border] = neutralColor.Shade(0.1f).Translucency(0.5f);

		colors[ImGuiCol_Text]         = whiteTextColor;
		colors[ImGuiCol_TextDisabled] = whiteTextColor.Shade(0.15f);

		colors[ImGuiCol_ModalWindowDimBg] = primaryColor.Shade(0.5f).Translucency(0.05f);

		PushButtonColor(GetNeutralColor(5));
		PushFrameBgColor(GetNeutralColor(3));
		PushHeaderColor();

		LoadFonts();
		Style::SetDefaultFont("WorkSans");
	}

	void PopGeneralStyle() {}

	// Make the UI compact because there are so many fields
	void PushStyleCompact()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		UI::PushStyleVar(ImGuiStyleVar_FramePadding,
		    ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
		UI::PushStyleVar(ImGuiStyleVar_ItemSpacing,
		    ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
	}

	void PopStyleCompact()
	{
		UI::PopStyleVar(2);
	}

	void PushFrameBgColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_FrameBg, color.Shade(0.5f));
		UI::PushStyleColor(ImGuiCol_FrameBgHovered, Hovered(color));
		UI::PushStyleColor(ImGuiCol_FrameBgActive, color);
	}

	void PopFrameBgColor()
	{
		UI::PopStyleColor(3);
	}

	void PushButtonColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_Button, color.Shade(0.3f));
		UI::PushStyleColor(ImGuiCol_ButtonHovered, Hovered(color));
		UI::PushStyleColor(ImGuiCol_ButtonActive, color);
	}

	void PopButtonColor()
	{
		UI::PopStyleColor(3);
	}

	void PushHeaderColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_Header, color.Shade(0.3f));
		UI::PushStyleColor(ImGuiCol_HeaderHovered, Hovered(color));
		UI::PushStyleColor(ImGuiCol_HeaderActive, color);
	}

	void PopHeaderColor()
	{
		UI::PopStyleColor(3);
	}

	void PushTextColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_Text, color);
		UI::PushStyleColor(ImGuiCol_TextDisabled, color.Shade(0.15f));
	}

	void PopTextColor()
	{
		UI::PopStyleColor(2);
	}
}    // namespace Rift::Style
