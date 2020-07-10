// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Platform/GenericPlatform.h"

#define PLATFORM_LINUX_USE_CHAR16 0


/**
 * Linux specific types
 **/
struct LinuxPlatformTypes : public GenericPlatformTypes
{
	using DWORD = unsigned int;
	using sizet = __SIZE_TYPE__;
	using TYPE_OF_NULL = decltype(__null);
#if PLATFORM_LINUX_USE_CHAR16
	using WIDECHAR = char16_t;
	using TCHAR = WIDECHAR;
#endif
};

using PlatformTypes = LinuxPlatformTypes;


#if BUILD_DEBUG
#	define FORCEINLINE inline
#else
#	define FORCEINLINE inline __attribute__((always_inline))
#endif	  // BUILD_DEBUG
#define NOINLINE __attribute__((noinline))

#if PLATFORM_LINUX_USE_CHAR16
#	undef PLATFORM_TCHAR_IS_CHAR16
#	define PLATFORM_TCHAR_IS_CHAR16 1
#endif