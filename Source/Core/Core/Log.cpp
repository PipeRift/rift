// Copyright 2015-2019 Piperift - All rights reserved

#include "Log.h"

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#if WITH_EDITOR
#include "UI/Windows/LogWindow.h"


template<typename Mutex>
class LogWindowSink : public spdlog::sinks::base_sink <Mutex>
{
public:

	LogWindowSink() : spdlog::sinks::base_sink<Mutex>()
	{
		set_pattern("%^[%T][%l]%$ %v");
	}

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		fmt::memory_buffer formatted;
		sink::formatter_->format(msg, formatted);

		if (LogWindow::globalLogWindow.IsValid())
		{
			LogWindow::globalLogWindow->Log(formatted.data(), formatted.size());
		}
	}

	void flush_() override {}
};
using LogWindowSink_mt = LogWindowSink<std::mutex>;
using LogWindowSink_st = LogWindowSink<spdlog::details::null_mutex>;
#endif
#include "Files/FileSystem.h"


#if TRACY_ENABLE
template<typename Mutex>
class ProfilerSink : public spdlog::sinks::base_sink<Mutex>
{
public:

	ProfilerSink() : spdlog::sinks::base_sink<Mutex>()
	{
		set_pattern("%^[%t][%l]%$ %v");
	}

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		// log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
		// msg.raw contains pre formatted log

		// If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
		fmt::memory_buffer formatted;
		sink::formatter_->format(msg, formatted);

		TracyMessage(formatted.data(), formatted.size()); // Send to profiler
	}

	void flush_() override {}
};
using ProfilerSink_mt = ProfilerSink<std::mutex>;
using ProfilerSink_st = ProfilerSink<spdlog::details::null_mutex>;
#endif


void Log::Init()
{
	// File
	const Path filePath = FileSystem::GetLogsPath() / "log.txt";
	auto fileSink{ std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		FileSystem::ToString(filePath).c_str(),
		1048576 * 5,
		3
	)};

	auto logger = std::make_shared<spdlog::logger>("Log", fileSink);
	logger->set_pattern("%^[%D %T][%t][%l]%$ %v");

	std::vector<spdlog::sink_ptr>& sinks = logger->sinks();
	sinks.reserve(5);

	// Console
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());

#if WITH_EDITOR
	// Log window
	sinks.push_back(std::make_shared<LogWindowSink_mt>());
#endif
#if TRACY_ENABLE
	// Profiler
	sinks.push_back(std::make_shared<ProfilerSink_mt>());
#endif
	spdlog::set_default_logger(logger);
}
