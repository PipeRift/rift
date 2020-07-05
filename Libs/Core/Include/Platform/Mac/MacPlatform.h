// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#define PLATFORM_MAC_USE_CHAR16 1

/**
 * Mac specific types
 **/
struct FMacPlatformTypes : public FGenericPlatformTypes
{
	using DWORD = unsigned int;
	using SIZE_T = size_t;
	using TYPE_OF_NULL = decltype(nullptr);
#if PLATFORM_MAC_USE_CHAR16
	using WIDECHAR = char16_t;
	using TCHAR = WIDECHAR;
#else
	using CHAR16 = char16_t;
#endif
};

typedef FMacPlatformTypes FPlatformTypes;


#if BUILD_DEBUG
#   define FORCEINLINE inline 												/* Don't force code to be inline */
#else
#   define FORCEINLINE inline __attribute__ ((always_inline))				/* Force code to be inline */
#endif
#define NOINLINE __attribute__((noinline))
