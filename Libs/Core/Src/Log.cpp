// Copyright 2015-2020 Piperift - All rights reserved

#include "Log.h"

#include "Files/FileSystem.h"

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


namespace VCLang::Log
{
#if TRACY_ENABLE
	template <typename Mutex>
	class ProfilerSink : public spdlog::sinks::base_sink<Mutex>
	{
		using Super = spdlog::sinks::base_sink<Mutex>;

	public:
		ProfilerSink() : Super()
		{
			Super::set_pattern("%^[%t][%l]%$ %v");
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			// log_msg is a struct containing the log entry info like level,
			// timestamp, thread id etc. msg.raw contains pre formatted log

			// If needed (very likely but not mandatory), the sink formats the
			// message before sending it to its final destination:
			spdlog::memory_buf_t formatted;
			Super::formatter_->format(msg, formatted);

			TracyMessage(formatted.data(), formatted.size());	 // Send to profiler
		}

		void flush_() override {}
	};
	using ProfilerSink_mt = ProfilerSink<std::mutex>;
	using ProfilerSink_st = ProfilerSink<spdlog::details::null_mutex>;
#endif


	void Init(Path logFile)
	{
		std::vector<spdlog::sink_ptr> sinks;
		sinks.reserve(3);

		// File
		if (!logFile.empty())
		{
			Path logFolder = logFile;
			if (FileSystem::IsFile(logFile))
			{
				logFolder.remove_filename();
			}
			else
			{
				logFile /= "log.txt";
			}
			FileSystem::CreateFolder(logFolder, true);

			sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
				FileSystem::ToString(logFile).c_str(), 1048576 * 5, 3));
		}

#if WITH_EDITOR
		// Log window
		sinks.push_back(std::make_shared<LogWindowSink_mt>());
#endif

#if TRACY_ENABLE
		// Profiler
		sinks.push_back(std::make_shared<ProfilerSink_mt>());
#endif

		// Console
		generalLogger = std::make_shared<spdlog::logger>("Log", sinks.begin(), sinks.end());
		errLogger = std::make_shared<spdlog::logger>("Log", sinks.begin(), sinks.end());
		generalLogger->set_pattern("%^[%D %T][%l]%$ %v");
		errLogger->set_pattern("%^[%D %T][%t][%l]%$ %v");

		auto cliSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		generalLogger->sinks().push_back(cliSink);
		cliSink->set_pattern("%^%v%$");

		auto cliErrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		errLogger->sinks().push_back(cliErrSink);
		cliErrSink->set_pattern("%^[%l] %v%$");

#if PLATFORM_WINDOWS
		cliSink->set_color(spdlog::level::info, cliSink->WHITE);
		cliErrSink->set_color(spdlog::level::warn, cliSink->YELLOW);
#else
		cliSink->set_color(spdlog::level::info, cliSink->white);
		cliErrSink->set_color(spdlog::level::warn, cliSink->yellow);
#endif
	}

	void Shutdown() {}

	void Info(const String& msg)
	{
		generalLogger->info(msg);
	}

	void Warning(const String& msg)
	{
		errLogger->warn(msg);
	}

	void Error(const String& msg)
	{
		errLogger->error(msg);
	}
}	 // namespace VCLang::Log
