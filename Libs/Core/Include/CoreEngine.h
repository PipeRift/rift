// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"

#include <utility>


// dllexport boilerplate
#if defined(CORE_DLL)
#	if defined(_MSC_VER)
#		if defined(CORE_COMPILE)
#			define CORE_API __declspec(dllexport)
#		else
#			define CORE_API __declspec(dllimport)
#		endif
#	elif defined(__GNUC__) || defined(__clang__)
#		if defined(CORE_COMPILE)
#			define CORE_API __attribute__((visibility("default")))
#		endif
#	endif
#endif

#if !defined(CORE_API)
#	define CORE_API
#endif

#define MoveTemp(x) std::move(x)
