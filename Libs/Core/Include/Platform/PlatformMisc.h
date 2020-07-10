// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <type_traits>


struct PlatformMisc
{
	static void CreateGuid(struct Guid& guid);
};


/**
 * Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.
 *
 * @param  Val        The value to align.
 * @param  Alignment  The alignment value, must be a power of two.
 *
 * @return The value aligned up to the specified alignment.
 */
template <typename T>
constexpr T Align(T Val, u64 Alignment)
{
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value,
		"Align expects an integer or pointer type");

	return (T)(((u64) Val + Alignment - 1) & ~(Alignment - 1));
}