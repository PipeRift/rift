// Copyright 2015-2023 Piperift - All rights reserved

#include "UI/Style.h"

#include "UI/Paths.h"
#include "UI/UIImgui.h"

#include <IconsFontAwesome5.h>
#include <Pipe/Core/Array.h>
#include <Pipe/Core/Checks.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Core/Tuples.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Math/Math.h>


namespace rift::UI
{
	using namespace p;


	struct FontType
	{
		TArray<TPair<float, ImFont*>> sizes{};

		void Add(float size, ImFont* imFont)
		{
			if (sizes.Contains([size](const auto& font) {
				    return math::NearlyEqual(font.first, size);
			    }))
			{
				p::Error(
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
				return math::NearlyEqual(font.first, desiredSize);
			});
			return foundFont ? foundFont->second : nullptr;
		}
	};

	struct FontDescriptor
	{
		std::array<FontType, GetEnumSize<UI::FontMode>()> modes{};

		FontType& operator[](UI::FontMode mode)
		{
			return modes[u8(mode)];
		}
		const FontType& operator[](UI::FontMode mode) const
		{
			return modes[u8(mode)];
		}
	};

	static TMap<Tag, FontDescriptor> gFonts{};


	ImFont* AddFont(Path file, float size, const ImFontConfig* fontConfig = nullptr,
	    const ImWchar* glyphRanges = nullptr)
	{
		auto& io = ImGui::GetIO();
		return io.Fonts->AddFontFromFileTTF(ToString(file).data(), size, fontConfig, glyphRanges);
	}

	void AddTextFont(Tag name, UI::FontMode mode, float size, p::Path file)
	{
		FontDescriptor* font = gFonts.Find(name);
		if (!font)
		{
			gFonts.Insert(name, {});
			font = &gFonts[name];
		}

		ImFont* imFont = AddFont(ToString(file).data(), size);
		(*font)[mode].Add(size, imFont);

		// Add Font Awesome icons
		static const ImWchar iconsRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
		ImFontConfig iconsConfig;
		iconsConfig.MergeMode        = true;
		iconsConfig.PixelSnapH       = true;
		iconsConfig.GlyphMinAdvanceX = 14.f;
		// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
		AddFont(file.parent_path() / FONT_ICON_FILE_NAME_FAS, 14.0f, &iconsConfig, iconsRanges);
	}

	void LoadFonts()
	{
		auto& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();

		auto resources = rift::Paths::GetResourcesPath() / "Editor";

		// Work Sans
		AddTextFont("WorkSans", UI::FontMode::Bold, 14.f, resources / "Fonts/WorkSans-Bold.ttf");
		AddTextFont("WorkSans", UI::FontMode::BoldItalic, 14.f,
		    resources / "Fonts/WorkSans-BoldItalic.ttf");
		AddTextFont(
		    "WorkSans", UI::FontMode::Italic, 14.f, resources / "Fonts/WorkSans-Italic.ttf");
		AddTextFont("WorkSans", UI::FontMode::Light, 14.f, resources / "Fonts/WorkSans-Light.ttf");
		AddTextFont("WorkSans", UI::FontMode::LightItalic, 14.f,
		    resources / "Fonts/WorkSans-LightItalic.ttf");
		AddTextFont(
		    "WorkSans", UI::FontMode::Regular, 14.f, resources / "Fonts/WorkSans-Regular.ttf");
		AddTextFont(
		    "WorkSans", UI::FontMode::Regular, 18.f, resources / "Fonts/WorkSans-Regular.ttf");

		// Karla
		AddTextFont("Karla", UI::FontMode::Bold, 14.f, resources / "Fonts/Karla-Bold.ttf");
		AddTextFont(
		    "Karla", UI::FontMode::BoldItalic, 14.f, resources / "Fonts/Karla-BoldItalic.ttf");
		AddTextFont("Karla", UI::FontMode::Italic, 14.f, resources / "Fonts/Karla-Italic.ttf");
		AddTextFont("Karla", UI::FontMode::Light, 14.f, resources / "Fonts/Karla-Light.ttf");
		AddTextFont(
		    "Karla", UI::FontMode::LightItalic, 14.f, resources / "Fonts/Karla-LightItalic.ttf");
		AddTextFont("Karla", UI::FontMode::Regular, 14.f, resources / "Fonts/Karla-Regular.ttf");

		io.Fonts->Build();
	}

	ImFont* FindFont(Tag name, UI::FontMode mode, float size)
	{
		const FontDescriptor* const font = gFonts.Find(name);
		return font ? (*font)[mode].Get(size) : nullptr;
	}

	void SetDefaultFont(Tag name, UI::FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			p::Error("Tried to set inexistent font '{}' as default", name);
		}
		ImGui::GetIO().FontDefault = font;
	}

	void PushFont(Tag name, UI::FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			p::Error("Tried to push inexistent font '{}' (mode: {}, size: {})", name,
			    GetEnumName(mode), size);
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

		LinearColor titleColor            = UI::GetNeutralColor(0);
		colors[ImGuiCol_TitleBg]          = titleColor.Shade(0.2f);
		colors[ImGuiCol_TitleBgActive]    = titleColor;
		colors[ImGuiCol_TitleBgCollapsed] = UI::Disabled(titleColor);

		colors[ImGuiCol_WindowBg] = UI::GetNeutralColor(1);
		colors[ImGuiCol_Border]   = UI::GetNeutralColor(0);

		colors[ImGuiCol_CheckMark]        = UI::whiteTextColor;
		colors[ImGuiCol_SliderGrabActive] = UI::GetNeutralColor(5);
		colors[ImGuiCol_SliderGrab]       = UI::GetNeutralColor(4);


		LinearColor separatorColor        = UI::GetNeutralColor(1);
		colors[ImGuiCol_SeparatorHovered] = UI::Hovered(separatorColor);
		colors[ImGuiCol_SeparatorActive]  = separatorColor;

		LinearColor resizeGripColor        = UI::GetNeutralColor(1);
		colors[ImGuiCol_ResizeGrip]        = resizeGripColor.Shade(0.3f);
		colors[ImGuiCol_ResizeGripHovered] = UI::Hovered(resizeGripColor);
		colors[ImGuiCol_ResizeGripActive]  = resizeGripColor;

		colors[ImGuiCol_DockingPreview] = UI::GetNeutralColor(2);
		colors[ImGuiCol_DockingEmptyBg] = LinearColor::White().Shade(0.97f);
		colors[ImGuiCol_TextSelectedBg] = UI::primaryColor.Shade(0.1f);

		colors[ImGuiCol_NavHighlight] = UI::primaryColor;

		// colors[ImGuiCol_Border] = neutralColor.Shade(0.1f).Translucency(0.5f);

		colors[ImGuiCol_Text]         = UI::whiteTextColor;
		colors[ImGuiCol_TextDisabled] = UI::whiteTextColor.Shade(0.15f);

		colors[ImGuiCol_ModalWindowDimBg] = UI::primaryColor.Shade(0.5f).Translucency(0.05f);

		colors[ImGuiCol_TableBorderStrong] = UI::GetNeutralColor(1).Shade(0.2f);
		colors[ImGuiCol_TableBorderLight]  = UI::GetNeutralColor(1).Shade(0.1f);

		UI::PushButtonColor(UI::GetNeutralColor(3));
		UI::PushFrameBgColor(UI::GetNeutralColor(2));
		UI::PushHeaderColor();

		LoadFonts();
		UI::SetDefaultFont("WorkSans");
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
		UI::PushStyleColor(ImGuiCol_FrameBg, color.Shade(0.3f));
		UI::PushStyleColor(ImGuiCol_FrameBgHovered, UI::Hovered(color));
		UI::PushStyleColor(ImGuiCol_FrameBgActive, color);
	}

	void PopFrameBgColor()
	{
		UI::PopStyleColor(3);
	}

	void PushButtonColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_Button, color);
		UI::PushStyleColor(ImGuiCol_ButtonHovered, UI::Hovered(color));
		UI::PushStyleColor(ImGuiCol_ButtonActive, color.Tint(0.1f));
	}

	void PopButtonColor()
	{
		UI::PopStyleColor(3);
	}

	void PushHeaderColor(LinearColor color)
	{
		UI::PushStyleColor(ImGuiCol_Header, color);
		UI::PushStyleColor(ImGuiCol_HeaderHovered, UI::Hovered(color));
		UI::PushStyleColor(ImGuiCol_HeaderActive, color.Tint(0.1f));
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
}    // namespace rift::UI
