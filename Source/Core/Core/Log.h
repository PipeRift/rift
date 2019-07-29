// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include <SDL_log.h>
#include <tracy/Tracy.hpp>

#include "Strings/String.h"
#include "Editor/Scene/LogWindow.h"

#if WITH_EDITOR
#endif

namespace Log {
	template<typename ...Args>
	inline void Message(const TCHAR* format, Args... args) {
		if (format)
		{
			if constexpr (sizeof...(args) > 0)
			{
				const String msg = CString::Printf(format, eastl::forward<Args>(args)...);
				TracyMessage(msg.c_str(), msg.size()); // Send to profiler
				SDL_Log(msg.c_str()); // Send to console

				#if WITH_EDITOR
				if (LogWindow::globalLogWindow.IsValid())
					LogWindow::globalLogWindow->Log(msg);
				#endif
			}
			else
			{
				const size_t size = std::strlen(format);
				TracyMessage(format, size); // Send to profiler
				SDL_Log(format); // Send to console

				#if WITH_EDITOR
				if (LogWindow::globalLogWindow.IsValid())
				{
					String msg{ format, size };
					LogWindow::globalLogWindow->Log(msg);
				}
				#endif
			}
		}
	}

	inline void Message(const String msg) {
		TracyMessage(msg.c_str(), msg.size()); // Send to profiler
		SDL_Log(msg.c_str()); // Send to console
	}

	template<typename ...Args>
	inline void Warning(const TCHAR* format, Args... args) {
		if constexpr (sizeof...(args) > 0)
		{
			String msg{ "Warning> " };
			msg.append_sprintf(format, eastl::forward<Args>(args)...);
			TracyMessage(msg.c_str(), msg.size()); // Send to profiler
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg.c_str()); // Send to console

			#if WITH_EDITOR
			if (LogWindow::globalLogWindow.IsValid())
				LogWindow::globalLogWindow->Log(msg);
			#endif
		}
		else
		{
			String msg{ "Warning> " };
			msg.append(format);
			TracyMessage(msg.c_str(), msg.size()); // Send to profiler
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg.c_str()); // Send to console

			#if WITH_EDITOR
			if (LogWindow::globalLogWindow.IsValid())
				LogWindow::globalLogWindow->Log(msg);
			#endif
		}
	}

	inline void Warning(const String msg) {
		Warning(msg.c_str());
	}

	template<typename ...Args>
	inline void Error(const TCHAR* format, Args... args) {
		if constexpr (sizeof...(args) > 0)
		{
			String msg{ "Error> " };
			msg.append_sprintf(format, eastl::forward<Args>(args)...);
			TracyMessage(msg.c_str(), msg.size()); // Send to profiler
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg.c_str()); // Send to console

			#if WITH_EDITOR
			if (LogWindow::globalLogWindow.IsValid())
				LogWindow::globalLogWindow->Log(msg);
			#endif
		}
		else
		{
			String msg{ "Error> " };
			msg.append(format);
			TracyMessage(msg.c_str(), msg.size()); // Send to profiler
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, msg.c_str()); // Send to console

			#if WITH_EDITOR
			if (LogWindow::globalLogWindow.IsValid())
				LogWindow::globalLogWindow->Log(msg);
			#endif
		}
	}

	inline void Error(const String msg) {
		Error(msg.c_str());
	}
};
