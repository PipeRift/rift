// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "Core/Platform/Platform.h"


class BitArray
{
private:
	/** The number of bits in this array */
	u32 arraySize;

	/** The number of uint32s for storing at least arraySize bits */
	u32 bitLongs;

	/** The array of uint32s containing the bits */
	u32 *bits;

public:

	/** Constructors */
	BitArray(u32 newSize);
	BitArray(BitArray&& other);
	BitArray(const BitArray& other);

	BitArray(u32 newSize, bool setClear);
	BitArray(u32 newSize, u32 *newBits);

	/** Destructor */
	~BitArray();

	BitArray& operator=(BitArray&& other);
	BitArray& operator=(const BitArray& other);
	BitArray  operator~();
	BitArray& operator^=(const BitArray& other);
	BitArray& operator&=(const BitArray& other);
	BitArray& operator|=(const BitArray& other);
	BitArray  operator^(const BitArray& other);
	BitArray  operator&(const BitArray& other);
	BitArray  operator|(const BitArray& other);

	/** @return true if a bit is set */
	inline bool IsValidIndex(u32 index) const
	{
		return bits[(index>>5)] >> (index & 0x0000001f) & 0x00000001;
	}

	/** Set all bits in this array */
	inline void FillBits()
	{
		FillBitArray(0xffffffff);
	}

	/** Set a single bit */
	inline void FillBit(u32 index)
	{
		bits[index >> 5] |= 0x00000001 << (index & 0x0000001f);
	}

	/** Clear all bits in this array */
	inline void ClearBits()
	{
		FillBitArray(0x00000000);
	}

	/** Clear a single bit */
	inline void ClearBit(u32 index)
	{
		bits[index >> 5] &= ~(0x00000001 << (index & 0x0000001f));
	}
	/** Fill with a 32-bit pattern */
	void FillBitArray(u32 pattern);

	/** flip a single bit.
	*/
	inline void FlipBit(u32 index)
	{
		if (IsValidIndex(index))
			ClearBit(index);
		else
			FillBit(index);
	};

	inline void Clear() {
		delete[]bits;
		arraySize = 0;
		bitLongs = 0;
		bits = nullptr;
	}

	/** Returns index of next set bit in array (wraps around)
	*/
	i32 GetNextSet(u32 index) const;

	/** @return index of previous set bit in array (wraps around) */
	i32 GetPreviousSet(u32 index) const;

	/** @return the number of bits in this bit array */
	u32 Size() { return arraySize; }
};
