// Copyright 2015-2019 Piperift - All rights reserved
// Based on Unreal Engine 4 Crc

#pragma once

#include "Core/Platform/Platform.h"


/**
 * CRC hash generation for different types of input data
 **/
struct Crc
{
	/** Lookup table with pre-calculated CRC values - slicing by 8 implementation */
	static u32 CRCTablesSB8[8][256];

	/** generates CRC hash of the memory area */
	static u32 MemCrc32( const void* Data, i32 Length, u32 CRC = 0);
};
