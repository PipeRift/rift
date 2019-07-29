// Copyright 2015-2019 Piperift - All rights reserved

#pragma once


#include "GenericPlatform.h"

#if PLATFORM_WINDOWS
	#include "Windows/WindowsPlatform.h"
#else
	#error Unknown platform
#endif

#include <GL/gl3w.h>



//------------------------------------------------------------------
// Transfer the platform types to global types
//------------------------------------------------------------------

//~ Unsigned base types.
/// An 8-bit unsigned integer.
typedef FPlatformTypes::uint8     u8;
/// A 16-bit unsigned integer.
typedef FPlatformTypes::uint16    u16;
/// A 32-bit unsigned integer.
typedef FPlatformTypes::uint32    u32;
/// A 64-bit unsigned integer.
typedef FPlatformTypes::uint64    u64;

//~ Signed base types.
/// An 8-bit signed integer.
typedef FPlatformTypes::int8      i8;
/// A 16-bit signed integer.
typedef FPlatformTypes::int16     i16;
/// A 32-bit signed integer.
typedef FPlatformTypes::int32     i32;
/// A 64-bit signed integer.
typedef FPlatformTypes::int64     i64;

//~ Character types.
/// An ANSI character. Normally a signed type.
typedef FPlatformTypes::ANSICHAR    ANSICHAR;
/// A wide character. Normally a signed type.
typedef FPlatformTypes::WIDECHAR    WIDECHAR;
/// Either ANSICHAR or WIDECHAR, depending on whether the platform supports wide characters or the requirements of the licensee.
typedef FPlatformTypes::TCHAR       TCHAR;
/// An 8-bit character containing a UTF8 (Unicode, 8-bit, variable-width) code unit.
typedef FPlatformTypes::CHAR8       UTF8CHAR;
/// A 16-bit character containing a UCS2 (Unicode, 16-bit, fixed-width) code unit, used for compatibility with 'Windows TCHAR' across multiple platforms.
typedef FPlatformTypes::CHAR16      UCS2CHAR;
/// A 16-bit character containing a UTF16 (Unicode, 16-bit, variable-width) code unit.
typedef FPlatformTypes::CHAR16      UTF16CHAR;
/// A 32-bit character containing a UTF32 (Unicode, 32-bit, fixed-width) code unit.
typedef FPlatformTypes::CHAR32      UTF32CHAR;

/// An unsigned integer the same size as a pointer
typedef FPlatformTypes::UPTRINT     UPTRINT;
/// A signed integer the same size as a pointer
typedef FPlatformTypes::PTRINT      PTRINT;
/// An unsigned integer the same size as a pointer, the same as UPTRINT
typedef FPlatformTypes::SIZE_T      SIZE_T;
/// An integer the same size as a pointer, the same as PTRINT
typedef FPlatformTypes::SSIZE_T     SSIZE_T;

/// The type of the NULL constant.
typedef FPlatformTypes::TYPE_OF_NULL    TYPE_OF_NULL;
/// The type of the C++ nullptr keyword.
typedef FPlatformTypes::TYPE_OF_NULLPTR	TYPE_OF_NULLPTR;


#if !defined(TX)
	#if PLATFORM_TCHAR_IS_1_BYTE
		#define TEXT_PASTE(x) x
	#elif PLATFORM_TCHAR_IS_CHAR16
		#define TEXT_PASTE(x) u ## x
	#elif PLATFORM_TCHAR_IS_WCHAR
		#define TEXT_PASTE(x) L ## x
	#else
		#define TEXT_PASTE(x) x
	#endif
	#define TX(x) TEXT_PASTE(x)
#endif