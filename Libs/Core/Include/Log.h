// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Files/FileSystem.h"
#include "Strings/String.h"

#include <spdlog/spdlog.h>

#include <Tracy.hpp>
#include <mutex>


namespace Log
{
	inline std::shared_ptr<spdlog::logger> generalLogger;
	inline std::shared_ptr<spdlog::logger> errLogger;


	void Init(Path logPath = {});
	void Shutdown();

	template <typename... Args>
	inline void Message(const TCHAR* format, Args... args)
	{
		if (format)
		{
			generalLogger->info(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Message(const String& msg)
	{
		generalLogger->info(msg);
	}

	template <typename... Args>
	inline void Warning(const TCHAR* format, Args... args)
	{
		if (format)
		{
			errLogger->warn(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Warning(const String& msg)
	{
		errLogger->warn(msg);
	}

	template <typename... Args>
	inline void Error(const TCHAR* format, Args... args)
	{
		if (format)
		{
			errLogger->error(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	inline void Error(const String& msg)
	{
		errLogger->error(msg);
	}
};	  // namespace Log
