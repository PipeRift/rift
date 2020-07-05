// Copyright 2015-2020 Piperift - All rights reserved
#pragma once


//---------------------------------------------------------------------
// Utility for automatically setting up the pointer-sized integer type
//---------------------------------------------------------------------

template<typename T32BITS, typename T64BITS, int PointerSize>
struct SelectIntPointerType
{
	// nothing here are is it an error if the partial specializations fail
};

template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 8>
{
	typedef T64BITS TIntPointer; // select the 64 bit type
};

template<typename T32BITS, typename T64BITS>
struct SelectIntPointerType<T32BITS, T64BITS, 4>
{
	typedef T32BITS TIntPointer; // select the 32 bit type
};


/**
 * Generic types for almost all compilers and platforms
 */
struct FGenericPlatformTypes
{
	// Unsigned base types.
	typedef unsigned char 		uint8;		// 8-bit  unsigned.
	typedef unsigned short int	uint16;		// 16-bit unsigned.
	typedef unsigned int		uint32;		// 32-bit unsigned.
	typedef unsigned long long	uint64;		// 64-bit unsigned.

	// Signed base types.
	typedef	signed char			int8;		// 8-bit  signed.
	typedef signed short int	int16;		// 16-bit signed.
	typedef signed int	 		int32;		// 32-bit signed.
	typedef signed long long	int64;		// 64-bit signed.

	// Character types.
	using ANSICHAR = char;	   // An ANSI character       -                  8-bit fixed-width representation of 7-bit characters.
	using WIDECHAR = wchar_t;  // A wide character        - In-memory only.  ?-bit fixed-width representation of the platform's natural wide character set.  Could be different sizes on different platforms.
	using CHAR8    = uint8;    // An 8-bit character type - In-memory only.  8-bit representation.  Should really be char8_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).
	using CHAR16   = uint16;   // A 16-bit character type - In-memory only.  16-bit representation.  Should really be char16_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).
	using CHAR32   = uint32;   // A 32-bit character type - In-memory only.  32-bit representation.  Should really be char32_t but making this the generic option is easier for compilers which don't fully support C++11 yet (i.e. MSVC).
	using TCHAR    = ANSICHAR; // A switchable character  - In-memory only.  Either ANSICHAR or WIDECHAR

	typedef SelectIntPointerType<uint32, uint64, sizeof(void*)>::TIntPointer UPTRINT;	// unsigned int the same size as a pointer
	typedef SelectIntPointerType<int32, int64, sizeof(void*)>::TIntPointer PTRINT;		// signed int the same size as a pointer
	typedef UPTRINT SIZE_T;																// unsigned int the same size as a pointer
	typedef PTRINT SSIZE_T;																// signed int the same size as a pointer

	typedef int32					TYPE_OF_NULL;
	typedef decltype(nullptr)		TYPE_OF_NULLPTR;
};

#ifndef PLATFORM_TCHAR_IS_1_BYTE
	#define PLATFORM_TCHAR_IS_1_BYTE			0
#endif
#ifndef PLATFORM_TCHAR_IS_4_BYTES
	#define PLATFORM_TCHAR_IS_4_BYTES			0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR16
	#define PLATFORM_TCHAR_IS_CHAR16			0
#endif
#ifndef PLATFORM_TCHAR_IS_WCHAR
	#define PLATFORM_TCHAR_IS_WCHAR		    	0
#endif