// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "Core/Platform/Platform.h"
#include "EASTL/internal/move_help.h"

// dllexport boilerplate
#if defined(CORE_DLL)
#   if defined(_MSC_VER)
#       if defined(LIBRARY_COMPILE)
#           define CORE_API __declspec(dllexport)
#       else
#           define CORE_API __declspec(dllimport)
#       endif
#   elif defined(__GNUC__) || defined(__clang__)
#       if defined(LIBRARY_COMPILE)
#           define CORE_API __attribute__ ((visibility ("default")))
#       endif
#   endif
#endif

#if !defined(CORE_API)
#   define CORE_API 
#endif

#define MoveTemp(x) eastl::move(x)
