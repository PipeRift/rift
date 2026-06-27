// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "UI/Style.h"

#include "UI/Paths.h"
#include "UI/UIImgui.h"

#include <IconsFontAwesome5.h>
#include <Pipe/Core/Checks.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/Templates.h>
#include <Pipe/Files/Paths.h>
#include <PipeContainers.h>
#include <PipeMath.h>
#include <PipeReflect.h>


namespace rift::UI
{
	struct FontType
	{
		p::TArray<p::TPair<float, ImFont*>> sizes{};

		void Add(float size, ImFont* imFont)
		{
			if (sizes.ContainsIf([size](const auto& font) {
				    return p::NearlyEqual(font.first, size);
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
			const p::TPair<float, ImFont*>* foundFont =
			    sizes.FindIf([desiredSize](const auto& font) {
				    return p::NearlyEqual(font.first, desiredSize);
			    });
			return foundFont ? foundFont->second : nullptr;
		}
	};

	struct FontDescriptor
	{
		std::array<FontType, p::GetEnumSize<UI::FontMode>()> modes{};

		FontType& operator[](UI::FontMode mode)
		{
			return modes[p::u8(mode)];
		}
		const FontType& operator[](UI::FontMode mode) const
		{
			return modes[p::u8(mode)];
		}
	};

	static p::TMap<p::Tag, FontDescriptor> gFonts{};


	ImFont* AddFont(p::StringView file, float size, const ImFontConfig* fontConfig = nullptr,
	    const ImWchar* glyphRanges = nullptr)
	{
		auto& io = ImGui::GetIO();
		return io.Fonts->AddFontFromFileTTF(file.data(), size, fontConfig, glyphRanges);
	}

	void AddTextFont(p::Tag name, UI::FontMode mode, float size, p::StringView file)
	{
		FontDescriptor* font = gFonts.Find(name);
		if (!font)
		{
			gFonts.Insert(name, {});
			font = &gFonts[name];
		}

		ImFont* imFont = AddFont(file.data(), size);
		(*font)[mode].Add(size, imFont);

		// Add Font Awesome icons
		static const ImWchar iconsRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
		ImFontConfig iconsConfig;
		iconsConfig.MergeMode        = true;
		iconsConfig.PixelSnapH       = true;
		iconsConfig.GlyphMinAdvanceX = 14.f;
		// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
		p::String path = p::JoinPaths(p::GetParentPath(file), FONT_ICON_FILE_NAME_FAS);
		AddFont(path, 14.0f, &iconsConfig, iconsRanges);
	}

	void LoadFonts()
	{
		auto& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();

		auto resources = p::JoinPaths(rift::Paths::GetResourcesPath(), "Editor");

		// Work Sans
		AddTextFont("WorkSans", UI::FontMode::Bold, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Bold.ttf"));
		AddTextFont("WorkSans", UI::FontMode::BoldItalic, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-BoldItalic.ttf"));
		AddTextFont("WorkSans", UI::FontMode::Italic, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Italic.ttf"));
		AddTextFont("WorkSans", UI::FontMode::Light, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Light.ttf"));
		AddTextFont("WorkSans", UI::FontMode::LightItalic, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-LightItalic.ttf"));
		AddTextFont("WorkSans", UI::FontMode::Regular, 14.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Regular.ttf"));
		AddTextFont("WorkSans", UI::FontMode::Regular, 18.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Regular.ttf"));
		AddTextFont("WorkSans", UI::FontMode::Regular, 24.f,
		    p::JoinPaths(resources, "Fonts/WorkSans-Regular.ttf"));

		// Karla
		AddTextFont(
		    "Karla", UI::FontMode::Bold, 14.f, p::JoinPaths(resources, "Fonts/Karla-Bold.ttf"));
		AddTextFont("Karla", UI::FontMode::BoldItalic, 14.f,
		    p::JoinPaths(resources, "Fonts/Karla-BoldItalic.ttf"));
		AddTextFont(
		    "Karla", UI::FontMode::Italic, 14.f, p::JoinPaths(resources, "Fonts/Karla-Italic.ttf"));
		AddTextFont(
		    "Karla", UI::FontMode::Light, 14.f, p::JoinPaths(resources, "Fonts/Karla-Light.ttf"));
		AddTextFont("Karla", UI::FontMode::LightItalic, 14.f,
		    p::JoinPaths(resources, "Fonts/Karla-LightItalic.ttf"));
		AddTextFont("Karla", UI::FontMode::Regular, 14.f,
		    p::JoinPaths(resources, "Fonts/Karla-Regular.ttf"));

		io.Fonts->Build();
	}

	ImFont* FindFont(p::Tag name, UI::FontMode mode, float size)
	{
		const FontDescriptor* const font = gFonts.Find(name);
		return font ? (*font)[mode].Get(size) : nullptr;
	}

	void SetDefaultFont(p::Tag name, UI::FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			p::Error("Tried to set inexistent font '{}' as default", name);
		}
		ImGui::GetIO().FontDefault = font;
	}

	void PushFont(p::Tag name, UI::FontMode mode, float size)
	{
		ImFont* font = FindFont(name, mode, size);
		if (!font && !name.IsNone())
		{
			p::Error("Tried to push inexistent font '{}' (mode: {}, size: {})", name,
			    p::GetEnumName(mode), size);
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

		p::LinearColor titleColor         = UI::GetNeutralColor(0);
		colors[ImGuiCol_TitleBg]          = titleColor.Shade(0.2f);
		colors[ImGuiCol_TitleBgActive]    = titleColor;
		colors[ImGuiCol_TitleBgCollapsed] = UI::ToDisabled(titleColor);

		colors[ImGuiCol_WindowBg] = UI::GetNeutralColor(1);
		colors[ImGuiCol_Border]   = UI::GetNeutralColor(0);

		colors[ImGuiCol_CheckMark]        = UI::whiteTextColor;
		colors[ImGuiCol_SliderGrabActive] = UI::GetNeutralColor(5);
		colors[ImGuiCol_SliderGrab]       = UI::GetNeutralColor(4);


		p::LinearColor separatorColor     = UI::GetNeutralColor(1);
		colors[ImGuiCol_SeparatorHovered] = UI::ToHovered(separatorColor);
		colors[ImGuiCol_SeparatorActive]  = separatorColor;

		p::LinearColor resizeGripColor     = UI::GetNeutralColor(1);
		colors[ImGuiCol_ResizeGrip]        = resizeGripColor.Shade(0.3f);
		colors[ImGuiCol_ResizeGripHovered] = UI::ToHovered(resizeGripColor);
		colors[ImGuiCol_ResizeGripActive]  = resizeGripColor;

		colors[ImGuiCol_DockingPreview] = UI::GetNeutralColor(2);
		colors[ImGuiCol_DockingEmptyBg] = p::LinearColor::White().Shade(0.97f);
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
		UI::PushHeaderColor(UI::GetNeutralColor(2));

		LoadFonts();
		UI::SetDefaultFont("WorkSans");
	}

	void PopGeneralStyle() {}
}    // namespace rift::UI
