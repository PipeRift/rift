// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Platform/GenericPlatform.h"

#include <cstddef>


#define PLATFORM_MAC_USE_CHAR16 0


/**
 * Mac specific types
 **/
struct MacPlatformTypes : public GenericPlatformTypes
{
	using DWORD = unsigned int;
	using sizet = std::size_t;
	using TYPE_OF_NULL = decltype(nullptr);
#if PLATFORM_MAC_USE_CHAR16
	using WIDECHAR = char16_t;
	using TCHAR = WIDECHAR;
#else
	using CHAR16 = char16_t;
#endif
};

using PlatformTypes = MacPlatformTypes;


#if BUILD_DEBUG
#	define FORCEINLINE inline /* Don't force code to be inline */
#else
#	define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#endif
#define NOINLINE __attribute__((noinline))

#if PLATFORM_MAC_USE_CHAR16
#	undef PLATFORM_TCHAR_IS_CHAR16
#	define PLATFORM_TCHAR_IS_CHAR16 1
#endif
