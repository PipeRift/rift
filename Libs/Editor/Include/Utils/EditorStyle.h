// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Tree.h>
#include <PMath/Color.h>
#include <PRefl/TypeId.h>
#include <UI/Style.h>


namespace rift::Style
{
	constexpr Color selectedColor{Color::FromHEX(0xdba43f)};
	constexpr Color executionColor{Color::FromHEX(0xdbdbdb)};

	constexpr Color functionColor{Color::FromHEX(0xCC3D33)};
	constexpr Color returnColor{functionColor.Shade(0.1f)};
	constexpr Color callColor{Color::FromHEX(0x3366CC)};
	constexpr Color flowColor{GetNeutralColor(4)};
	constexpr Color invalidColor{Color::FromHEX(0xF95040)};

	template<typename T>
	constexpr Color GetTypeColor()
	{
		if constexpr (IsSame<T, bool>)
		{
			return Color::FromHEX(0xF94144);
		}
		else if constexpr (FloatingPoint<T>)
		{
			return Color::FromHEX(0x90BE6D);
		}
		else if constexpr (SignedIntegral<T>)
		{
			return Color::FromHEX(0x43AA8B);
		}
		else if constexpr (UnsignedIntegral<T>)
		{
			return Color::FromHEX(0x4D908E);
		}
		else if constexpr (IsSame<T, String>)
		{
			return Color::FromHEX(0xD44BFA);
		}
		return Color::Gray();
	};

	const Color GetTypeColor(const AST::Tree& ast, AST::Id id);


	void PushNodeTitleColor(Color color);
	void PopNodeTitleColor();

	void PushNodeBackgroundColor(Color color);
	void PopNodeBackgroundColor();
}    // namespace rift::Style
