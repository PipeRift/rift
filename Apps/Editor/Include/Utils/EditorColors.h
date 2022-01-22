// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Tree.h>
#include <Math/Color.h>
#include <Reflection/TypeId.h>


namespace Rift::Style
{
	constexpr Color selectedColor{Color::FromHEX(0xdba43f)};
	constexpr Color executionColor{Color::FromHEX(0xdbdbdb)};

	constexpr Color functionColor{Color::FromHEX(0xCC3D33)};
	constexpr Color callColor{Color::FromHEX(0x3366CC)};

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
}    // namespace Rift::Style
