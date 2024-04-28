// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include <AST/Tree.h>
#include <Pipe/Reflect/TypeId.h>
#include <PipeColor.h>
#include <UI/Style.h>


namespace rift::editor
{
	using namespace p;


	constexpr Color selectedColor{Color::FromHex(0xdba43f)};
	constexpr Color executionColor{Color::FromHex(0xdbdbdb)};

	constexpr Color functionColor{Color::FromHex(0xCC3D33)};
	constexpr Color returnColor{functionColor.Shade(0.1f)};
	constexpr Color callColor{Color::FromHex(0x3366CC)};
	constexpr Color flowColor{UI::GetNeutralColor(4)};
	constexpr Color invalidColor{Color::FromHex(0xF95040)};

	template<typename T>
	constexpr Color GetTypeColor()
	{
		if constexpr (IsSame<T, bool>)
		{
			return Color::FromHex(0xBF4A41);
		}
		else if constexpr (FloatingPoint<T>)
		{
			return Color::FromHex(0x54BFA6);
		}
		else if constexpr (SignedIntegral<T>)
		{
			return Color::FromHex(0x63BF54);
		}
		else if constexpr (UnsignedIntegral<T>)
		{
			return Color::FromHex(0x54BF79);
		}
		else if constexpr (IsSame<T, String>)
		{
			return Color::FromHex(0xBF54AE);
		}
		else if constexpr (IsSame<T, Class>)
		{
			return Color::FromHex(0x545FBF);
		}
		else if constexpr (IsSame<T, Struct>)
		{
			return Color::FromHex(0x548CBF);
		}
		return Color::Gray();
	};

	const Color GetTypeColor(const ast::Tree& ast, ast::Id id);


	void PushNodeTitleColor(Color color);
	void PopNodeTitleColor();

	void PushNodeBackgroundColor(Color color);
	void PopNodeBackgroundColor();
}    // namespace rift::editor
