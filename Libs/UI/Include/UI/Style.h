// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <PipeColor.h>


namespace rift::UI
{
	using namespace p;


	constexpr LinearColor primaryColor = LinearColor::FromHEX(0xD6863B);

	constexpr LinearColor whiteTextColor = LinearColor::White().Shade(0.05f);
	constexpr LinearColor blackTextColor = LinearColor::Black().Tint(0.05f);

	constexpr LinearColor infoColor    = LinearColor::FromHEX(0x2B8ED6);
	constexpr LinearColor successColor = LinearColor::FromHEX(0x4DD62B);
	constexpr LinearColor warningColor = LinearColor::FromHEX(0xD6AB2B);
	constexpr LinearColor errorColor   = LinearColor::FromHEX(0xD62B2B);


	constexpr LinearColor GetNeutralColor(u8 level)
	{
		switch (level)
		{
			default:
			case 0: return LinearColor::FromHEX(0x222222);
			case 1: return LinearColor::FromHEX(0x3B3B3B);
			case 2: return LinearColor::FromHEX(0x464646);
			case 3: return LinearColor::FromHEX(0x515151);
			case 4: return LinearColor::FromHEX(0x626262);
			case 5: return LinearColor::FromHEX(0x6E6E6E);
			case 6: return LinearColor::FromHEX(0x9E9E9E);
			case 7: return LinearColor::FromHEX(0xB1B1B1);
			case 8: return LinearColor::FromHEX(0xCFCFCF);
			case 9: return LinearColor::FromHEX(0xE1E1E1);
			case 10: return LinearColor::FromHEX(0xF7F7F7);
		}
	}
	inline LinearColor GetNeutralTextColor(u8 level)
	{
		return level <= 5 ? whiteTextColor : blackTextColor;
	}


	enum class FontMode : p::u8
	{
		None = 0,
		Regular,
		Bold,
		Light,
		Italic,
		BoldItalic,
		LightItalic,
	};

	// Sets the default font
	// @param name of the font
	// @param mode of the font. E.g: Bold
	// @param size desired size of the font. If 0, first size available is used
	void SetDefaultFont(p::Tag name, FontMode mode = FontMode::Regular, float size = 0.f);

	// Pushes a font
	// @param name of the font
	// @param mode of the font. E.g: Bold
	// @param size desired size of the font. If 0, first size available is used
	void PushFont(p::Tag name, FontMode mode = FontMode::Regular, float size = 0.f);

	void PopFont();

	void PushGeneralStyle();
	void PopGeneralStyle();

	void PushStyleCompact();
	void PopStyleCompact();

	void PushFrameBgColor(LinearColor color);
	void PopFrameBgColor();
	void PushButtonColor(LinearColor color);
	void PopButtonColor();
	void PushHeaderColor(LinearColor color = GetNeutralColor(2));
	void PopHeaderColor();

	void PushTextColor(LinearColor color);
	void PopTextColor();

	template<p::ColorMode mode>
	TColor<mode> Hovered(const TColor<mode>& color)
	{
		return color.Shade(0.1f);
	}

	template<ColorMode mode>
	TColor<mode> Disabled(const TColor<mode>& color)
	{
		return color.Shade(0.2f);
	}
};    // namespace rift::UI
