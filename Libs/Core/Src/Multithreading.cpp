// Copyright 2015-2020 Piperift - All rights reserved

#include "Multithreading.h"

#include "Math/Math.h"
#include "Strings/String.h"

#include <common/TracySystem.hpp>


TaskSystem::TaskSystem()
{
	// Prefer 1, but don't exceed max threads
	const u32 gamePoolSize = Math::Min(1u, std::thread::hardware_concurrency());
	// Prefer max threads - main threads, but don't go under 1
	const u32 workerPoolSize = Math::Max(1u, std::thread::hardware_concurrency() - gamePoolSize);

	gamePool = std::make_shared<ThreadPool>(gamePoolSize);
	workerPool = std::make_shared<ThreadPool>(workerPoolSize);

	// Name game thread
	tracy::SetThreadName("Game");

	TaskFlow flow;
	flow.parallel_for(0, i32(GetNumWorkerThreads()) - 1, 1, [](i32 i) {
		// Name each worker thread in the debugger
		tracy::SetThreadName(CString::Format("Worker {}", i + 1).c_str());
	});
	RunFlow(flow);
}
