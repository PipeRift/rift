// Copyright 2015-2019 Piperift - All rights reserved

#include "Multithreading.h"
#include <tracy/common/TracySystem.hpp>

#include "Math/Math.h"
#include "Strings/String.h"


TaskSystem::TaskSystem()
{
	// Prefer 1, but don't exceed max threads
	const u32 gamePoolSize = Math::Min(1u, std::thread::hardware_concurrency());
	// Prefer max threads - main threads, but don't go under 1
	const u32 workerPoolSize = Math::Max(1u, std::thread::hardware_concurrency() - gamePoolSize);

	gamePool = std::make_shared<ThreadPool>(gamePoolSize);
	workerPool = std::make_shared<ThreadPool>(workerPoolSize);

	// Name game thread
	tracy::SetThreadName(gamePool->_threads[0], "Game");

	for (size_t i = 0; i < workerPool->_threads.size(); ++i)
	{
		// Name each worker thread in the debugger
		tracy::SetThreadName(workerPool->_threads[i], CString::Printf("Worker %i", i + 1).c_str());
	}
}
