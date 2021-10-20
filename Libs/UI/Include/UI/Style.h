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
	inline LinearColor primaryColor       = Color::HexRGB(0xbc5b00);
	inline LinearColor primaryTextColor   = Color::White.Darken(0.05f);
	inline LinearColor secondaryColor     = Color::HexRGB(0xD56A38);
	inline LinearColor secondaryTextColor = Color::Black.Lighten(0.1f);

	inline LinearColor fillColor     = Color::HexRGB(0x888888);
	inline LinearColor fillTextColor = Color::White.Darken(0.08f);


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
	void PushHeaderColor(LinearColor color);
	void PopHeaderColor();

	void PushTextColor(LinearColor color);
	void PopTextColor();

	LinearColor Hovered(const LinearColor& color);
	LinearColor Disabled(const LinearColor& color);
};
