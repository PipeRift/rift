// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

/**
* Windows specific types
**/
struct FWindowsPlatformTypes : public FGenericPlatformTypes
{
#ifdef _WIN64
	typedef unsigned __int64	SIZE_T;
	typedef __int64				SSIZE_T;
#else
	typedef unsigned long		SIZE_T;
	typedef long				SSIZE_T;
#endif
};

typedef FWindowsPlatformTypes FPlatformTypes;


#define FORCEINLINE __forceinline /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */
