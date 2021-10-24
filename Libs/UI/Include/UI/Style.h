// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Math/Color.h>
#include <Platform/Platform.h>
#include <Reflection/Static/EnumType.h>


namespace Rift::Style
{
	const LinearColor primaryColor = LinearColor::Hex(0xD6863B);

	const LinearColor whiteTextColor = LinearColor::White().Shade(0.05f);
	const LinearColor blackTextColor = LinearColor::Black().Tint(0.05f);


	inline LinearColor GetNeutralColor(u8 level)
	{
		switch (level)
		{
			default:
			case 0: return LinearColor::Hex(0x222222);
			case 1: return LinearColor::Hex(0x3B3B3B);
			case 2: return LinearColor::Hex(0x464646);
			case 3: return LinearColor::Hex(0x515151);
			case 4: return LinearColor::Hex(0x626262);
			case 5: return LinearColor::Hex(0x6E6E6E);
			case 6: return LinearColor::Hex(0x9E9E9E);
			case 7: return LinearColor::Hex(0xB1B1B1);
			case 8: return LinearColor::Hex(0xCFCFCF);
			case 9: return LinearColor::Hex(0xE1E1E1);
			case 10: return LinearColor::Hex(0xF7F7F7);
		}
	}
	inline LinearColor GetNeutralTextColor(u8 level)
	{
		return level <= 5 ? whiteTextColor : blackTextColor;
	}


	enum class FontMode : u8
	{
		Regular,
		Bold,
		Light,
		Italic,
		BoldItalic,
		LightItalic,
		None
	};

	// Sets the default font
	// @param name of the font
	// @param mode of the font. E.g: Bold
	// @param size desired size of the font. If 0, first size available is used
	void SetDefaultFont(Rift::Name name, FontMode mode = FontMode::Regular, float size = 0.f);

	// Pushes a font
	// @param name of the font
	// @param mode of the font. E.g: Bold
	// @param size desired size of the font. If 0, first size available is used
	void PushFont(Rift::Name name, FontMode mode = FontMode::Regular, float size = 0.f);

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

	template<ColorMode mode>
	TColor<mode> Hovered(const TColor<mode>& color)
	{
		return color.Shade(0.1f);
	}

	template<ColorMode mode>
	TColor<mode> Disabled(const TColor<mode>& color)
	{
		return color.Shade(0.3f);
	}
};
