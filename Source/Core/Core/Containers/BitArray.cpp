// Copyright 2015-2019 Piperift - All rights reserved

#include "BitArray.h"

BitArray::BitArray(u32 newSize)
{
	arraySize = newSize;
	bitLongs  = ((arraySize - 1) >> 5) + 1;
	bits = new u32[bitLongs];
}

BitArray::BitArray(BitArray&& other)
{
	arraySize = other.arraySize;
	bitLongs  = other.bitLongs;
	bits = other.bits;

	other.arraySize = 0;
	other.bitLongs = 0;
	other.bits = nullptr;
}

BitArray::BitArray(const BitArray& other)
{
	arraySize = other.arraySize;
	bitLongs  = other.bitLongs;
	bits = new u32[bitLongs];
	for (u32 i = 0; i < bitLongs; i++)
		bits[i] = other.bits[i];
}

BitArray::BitArray(u32 newSize, bool bitValues)
{
	arraySize = newSize;
	bitLongs = ((arraySize - 1) >> 5) + 1;
	bits = new u32[bitLongs];

	if (bitValues)
		FillBits();
	else
		ClearBits();
}

BitArray::BitArray(u32 newSize, u32 *newBits)
{
	arraySize = newSize;
	bitLongs = ((arraySize - 1) >> 5) + 1;
	bits = new u32[bitLongs];
	for (u32 i = 0; i < bitLongs; i++)
		bits[i] = newBits[i];
}

BitArray::~BitArray()
{
	delete []bits;
}

BitArray& BitArray::operator=(BitArray&& other)
{
	// Clear previous bits
	delete[]bits;

	arraySize = other.arraySize;
	bitLongs = other.bitLongs;
	bits = other.bits;

	other.arraySize = 0;
	other.bitLongs = 0;
	other.bits = nullptr;

	return *this;
}
BitArray& BitArray::operator=(const BitArray& other)
{
	bool equalSize = arraySize == other.arraySize;

	arraySize = other.arraySize;
	bitLongs = other.bitLongs;

	if (!equalSize) {
		delete []bits;
		bits = new u32[bitLongs];
	}
	for (u32 i = 0; i < bitLongs; i++)
		bits[i] = other.bits[i];

	return *this;
}

BitArray BitArray::operator~()
{
	BitArray result(arraySize);

	for (u32 i = 0; i < bitLongs; i++)
		result.bits[i] = ~bits[i];

	return result;
}

BitArray& BitArray::operator^=(const BitArray& other)
{
	for (u32 i = 0; i < ((bitLongs < other.bitLongs) ? bitLongs : other.bitLongs); i++)
		bits[i] ^= other.bits[i];
	return *this;
}

BitArray& BitArray::operator&=(const BitArray& other)
{
	for (u32 i = 0; i < ((bitLongs < other.bitLongs) ? bitLongs : other.bitLongs); i++)
		bits[i] &= other.bits[i];
	return *this;
}

BitArray& BitArray::operator|=(const BitArray& other)
{
	for (u32 i = 0; i < (bitLongs < other.bitLongs ? bitLongs : other.bitLongs); i++)
		bits[i] |= other.bits[i];
	return *this;
}

BitArray BitArray::operator^(const BitArray& other)
{
	BitArray result((arraySize < other.arraySize) ? arraySize : other.arraySize);

	for (u32 i = 0; i < result.bitLongs; i++)
		result.bits[i] = bits[i] ^ other.bits[i];
	return result;
}

BitArray BitArray::operator&(const BitArray& other)
{
	BitArray result((arraySize < other.arraySize) ? arraySize : other.arraySize);

	for (u32 i = 0; i < result.bitLongs; i++)
		result.bits[i] = bits[i] & other.bits[i];
	return result;
}

BitArray BitArray::operator|(const BitArray& other)
{
	BitArray result((arraySize < other.arraySize) ? arraySize : other.arraySize);

	for (u32 i = 0; i < result.bitLongs; i++)
		result.bits[i] = bits[i] | other.bits[i];
	return result;
}

void BitArray::FillBitArray(u32 pattern)
{
	for (u32 i = 0; i < bitLongs; bits[i++] = pattern);
}

i32 BitArray::GetNextSet(u32 index) const
{
	u32 i;
	for (i = index + 1; i < arraySize; i++)
	{
		if (IsValidIndex(i))
			return i;
	}

	for (i = 0; i < index - 1; i++)
	{
		if (IsValidIndex(i))
			return i;
	}
	return -1;
}

i32 BitArray::GetPreviousSet(u32 index) const
{
	u32 i;
	if (index != 0)
	{
		for (i = index - 1; i > 0; i--)
		{
			if (IsValidIndex(i))
				return i;
		}
		if (IsValidIndex(0))
			return 0;
	}

	for (i = arraySize - 1; i > index; i--)
	{
		if (IsValidIndex(i))
			return i;
	}
	return -1;
}
