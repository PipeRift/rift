// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Files/FileSystem.h"
#include "Strings/String.h"

#include <Tracy.hpp>
#include <mutex>


namespace spdlog
{
	class logger;
}

namespace Log
{
	inline std::shared_ptr<spdlog::logger> generalLogger;
	inline std::shared_ptr<spdlog::logger> errLogger;


	void Init(Path logPath = {});
	void Shutdown();

	void Info(const String& msg);
	void Warning(const String& msg);
	void Error(const String& msg);

	template <typename... Args>
	void Info(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Info(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	void Warning(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Warning(CString::Format(format, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	void Error(StringView format, Args... args)
	{
		if (!format.empty())
		{
			Error(CString::Format(format, std::forward<Args>(args)...));
		}
	}
};	  // namespace Log
