// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Strings/String.h"
#include "Files/FileSystem.h"

#include <spdlog/spdlog.h>

#include <Tracy.hpp>
#include <mutex>


namespace Log
{
	void Init(Path logPath = {});

	template <typename... Args>
	inline void Message(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::info(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Message(const String& msg)
	{
		spdlog::info(msg);
	}

	template <typename... Args>
	inline void Warning(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::warn(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Warning(const String& msg)
	{
		spdlog::warn(msg);
	}

	template <typename... Args>
	inline void Error(const TCHAR* format, Args... args)
	{
		if (format)
		{
			spdlog::error(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Error(const String& msg)
	{
		spdlog::error(msg);
	}
};	  // namespace Log
