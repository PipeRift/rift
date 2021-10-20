// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <Math/Color.h>
#include <Reflection/TypeId.h>


namespace Rift::Graph
{
	constexpr Color selectedColor{Color::HexRGB(0xdba43f)};

	template <typename T>
	constexpr Color GetTypeColor()
	{
		if (IsSame<T, bool>)
		{
			return Color::HexRGB(0xF94144);
		}
		else if (FloatingPoint<T>)
		{
			return Color::HexRGB(0x90BE6D);
		}
		else if (SignedIntegral<T>)
		{
			return Color::HexRGB(0x43AA8B);
		}
		else if (UnsignedIntegral<T>)
		{
			return Color::HexRGB(0x4D908E);
		}
		else if (IsSame<T, String>)
		{
			return Color::HexRGB(0xD44BFA);
		}
		return Color::Gray;
	};

	const Color GetHovered(Color source)
	{
		return source.Darken(0.3f);
	}
}    // namespace Rift::Graph
