// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <Math/Color.h>
#include <Reflection/TypeId.h>


namespace Rift::Graph
{
	constexpr Color selectedColor{Color::FromHEX(0xdba43f)};
	constexpr Color executionColor{Color::FromHEX(0xdbdbdb)};

	template<typename T>
	constexpr Color GetTypeColor()
	{
		if (IsSame<T, bool>)
		{
			return Color::FromHEX(0xF94144);
		}
		else if (FloatingPoint<T>)
		{
			return Color::FromHEX(0x90BE6D);
		}
		else if (SignedIntegral<T>)
		{
			return Color::FromHEX(0x43AA8B);
		}
		else if (UnsignedIntegral<T>)
		{
			return Color::FromHEX(0x4D908E);
		}
		else if (IsSame<T, String>)
		{
			return Color::FromHEX(0xD44BFA);
		}
		return Color::Gray();
	};

	const Color GetHovered(Color source)
	{
		return source.Shade(0.3f);
	}
}    // namespace Rift::Graph
