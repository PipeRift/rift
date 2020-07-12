// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Platform/Windows/WindowsPlatform.h"
#elif PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatform.h"
#elif PLATFORM_MAC
#	include "Platform/Mac/MacPlatform.h"
#else
#	error Unknown platform
#endif


namespace VCLang
{
	//------------------------------------------------------------------
	// Transfer the platform types to global types
	//------------------------------------------------------------------

	//~ Unsigned base types.
	/// An 8-bit unsigned integer.
	typedef PlatformTypes::uint8 u8;
	/// A 16-bit unsigned integer.
	typedef PlatformTypes::uint16 u16;
	/// A 32-bit unsigned integer.
	typedef PlatformTypes::uint32 u32;
	/// A 64-bit unsigned integer.
	typedef PlatformTypes::uint64 u64;

	//~ Signed base types.
	/// An 8-bit signed integer.
	typedef PlatformTypes::int8 i8;
	/// A 16-bit signed integer.
	typedef PlatformTypes::int16 i16;
	/// A 32-bit signed integer.
	typedef PlatformTypes::int32 i32;
	/// A 64-bit signed integer.
	typedef PlatformTypes::int64 i64;

	//~ Character types.
	/// An ANSI character. Normally a signed type.
	typedef PlatformTypes::ANSICHAR ANSICHAR;
	/// A wide character. Normally a signed type.
	typedef PlatformTypes::WIDECHAR WIDECHAR;
	/// Either ANSICHAR or WIDECHAR, depending on whether the platform supports wide
	/// characters or the requirements of the licensee.
	typedef PlatformTypes::TCHAR TCHAR;
	/// An 8-bit character containing a UTF8 (Unicode, 8-bit, variable-width) code
	/// unit.
	typedef PlatformTypes::CHAR8 UTF8CHAR;
	/// A 16-bit character containing a UCS2 (Unicode, 16-bit, fixed-width) code
	/// unit, used for compatibility with 'Windows TCHAR' across multiple platforms.
	typedef PlatformTypes::CHAR16 UCS2CHAR;
	/// A 16-bit character containing a UTF16 (Unicode, 16-bit, variable-width) code
	/// unit.
	typedef PlatformTypes::CHAR16 UTF16CHAR;
	/// A 32-bit character containing a UTF32 (Unicode, 32-bit, fixed-width) code
	/// unit.
	typedef PlatformTypes::CHAR32 UTF32CHAR;

	/// An unsigned integer the same size as a pointer
	using uPtr = PlatformTypes::uPtr;
	/// A signed integer the same size as a pointer
	using iPtr = PlatformTypes::iPtr;
	/// An unsigned integer the same size as a pointer, the same as UPTRINT
	using sizet = PlatformTypes::sizet;
	/// An integer the same size as a pointer, the same as PTRINT
	using ssizet = PlatformTypes::ssizet;

	/// The type of the NULL constant.
	typedef PlatformTypes::TYPE_OF_NULL TYPE_OF_NULL;
	/// The type of the C++ nullptr keyword.
	typedef PlatformTypes::TYPE_OF_NULLPTR TYPE_OF_NULLPTR;
}	 // namespace VCLang

#if !defined(TX)
#	if PLATFORM_TCHAR_IS_1_BYTE
#		define TEXT_PASTE(x) x
#	elif PLATFORM_TCHAR_IS_CHAR16
#		define TEXT_PASTE(x) u##x
#	elif PLATFORM_TCHAR_IS_WCHAR
#		define TEXT_PASTE(x) L##x
#	else
#		define TEXT_PASTE(x) x
#	endif
#	define TX(x) TEXT_PASTE(x)
#endif
