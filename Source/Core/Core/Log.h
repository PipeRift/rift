// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <mutex>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "Strings/String.h"


namespace Log
{
	void Init();

	template<typename ...Args>
	inline void Message(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::info(format, eastl::forward<Args>(args)...);
		}
	}

	inline void Message(const String& msg)
	{
		spdlog::info(msg.data());
	}

	template<typename ...Args>
	inline void Warning(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::warn(format, eastl::forward<Args>(args)...);
		}
	}

	inline void Warning(const String& msg)
	{
		spdlog::warn(msg.data());
	}

	template<typename ...Args>
	inline void Error(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::error(format, eastl::forward<Args>(args)...);
		}
	}

	inline void Error(const String& msg)
	{
		spdlog::error(msg.data());
	}
};
