// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <Pipe/Core/Tag.h>
#include <PipeColor.h>


namespace rift::UI
{
	constexpr p::LinearColor primaryColor = p::LinearColor::FromHex(0xD6863B);

	constexpr p::LinearColor whiteTextColor = p::LinearColor::White().Shade(0.05f);
	constexpr p::LinearColor blackTextColor = p::LinearColor::Black().Tint(0.05f);

	constexpr p::LinearColor infoColor    = p::LinearColor::FromHex(0x2B8ED6);
	constexpr p::LinearColor successColor = p::LinearColor::FromHex(0x4DD62B);
	constexpr p::LinearColor warningColor = p::LinearColor::FromHex(0xD6AB2B);
	constexpr p::LinearColor errorColor   = p::LinearColor::FromHex(0xD62B2B);


	constexpr p::LinearColor GetNeutralColor(p::u8 level)
	{
		switch (level)
		{
			default:
			case 0: return p::LinearColor::FromHex(0x222222);
			case 1: return p::LinearColor::FromHex(0x3B3B3B);
			case 2: return p::LinearColor::FromHex(0x464646);
			case 3: return p::LinearColor::FromHex(0x515151);
			case 4: return p::LinearColor::FromHex(0x626262);
			case 5: return p::LinearColor::FromHex(0x6E6E6E);
			case 6: return p::LinearColor::FromHex(0x9E9E9E);
			case 7: return p::LinearColor::FromHex(0xB1B1B1);
			case 8: return p::LinearColor::FromHex(0xCFCFCF);
			case 9: return p::LinearColor::FromHex(0xE1E1E1);
			case 10: return p::LinearColor::FromHex(0xF7F7F7);
		}
	}
	inline p::LinearColor GetNeutralTextColor(p::u8 level)
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
};    // namespace rift::UI
