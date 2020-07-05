// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#define PLATFORM_LINUX_USE_CHAR16 1

/**
* Linux specific types
**/
struct FLinuxPlatformTypes : public FGenericPlatformTypes
{
	using DWORD = unsigned int;
	using SIZE_T = __SIZE_TYPE__;
	using TYPE_OF_NULL = decltype(__null);
#if PLATFORM_LINUX_USE_CHAR16
	using WIDECHAR = char16_t;
	using TCHAR = WIDECHAR;
#endif
};

typedef FLinuxPlatformTypes FPlatformTypes;


#if BUILD_DEBUG
	#define FORCEINLINE inline
#else
	#define FORCEINLINE inline __attribute__ ((always_inline))
#endif // BUILD_DEBUG
#define NOINLINE __attribute__((noinline))
