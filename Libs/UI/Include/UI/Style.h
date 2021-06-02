// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Math/Color.h>
#include <Platform/Platform.h>
#include <Reflection/Static/EnumType.h>


namespace Rift::Style
{
	enum class FontMode : Rift::u8
	{
		Regular,
		Bold,
		Light,
		Italic,
		BoldItalic,
		LightItalic,
		None
	};

	enum class ThemeColors : Rift::u8
	{
		Primary,
		Secondary,
		Fill
	};
	inline LinearColor primaryColor       = Color{0, 186, 189};
	inline LinearColor primaryTextColor   = LinearColor{Color::Black}.Lighten(0.1f);
	inline LinearColor secondaryColor     = Color{213, 106, 56};
	inline LinearColor secondaryTextColor = LinearColor{Color::Black}.Lighten(0.1f);

	inline LinearColor fillColor     = Color{136, 136, 140};
	inline LinearColor fillTextColor = LinearColor{Color::White}.Darken(0.15f);


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
};
