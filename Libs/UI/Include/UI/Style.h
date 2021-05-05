// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

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

	void ApplyStyle();

	void PushStyleCompact();
	void PopStyleCompact();
};
