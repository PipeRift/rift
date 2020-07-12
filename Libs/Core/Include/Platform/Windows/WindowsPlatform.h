// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Platform/GenericPlatform.h"


namespace VCLang
{
	/**
	 * Windows specific types
	 **/
	struct WindowsPlatformTypes : public GenericPlatformTypes
	{
#ifdef _WIN64
		using sizet = __int64;
		using ssizet = __int64;
#else
		using sizet = unsigned long;
		using ssizet = long;
#endif
	};

	using PlatformTypes = WindowsPlatformTypes;
}	 // namespace VCLang

#define FORCEINLINE __forceinline	  /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */
