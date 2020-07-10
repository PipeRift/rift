// Copyright 2015-2020 Piperift - All rights reserved

#include "Tasks.h"

#include "Context.h"
#include "Math/Math.h"
#include "Profiler.h"
#include "Strings/String.h"

#include <chrono>
#include <common/TracySystem.hpp>
#include <condition_variable>
#include <mutex>


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


	// Go over all worker threads naming them and making sure no thread is repeated
	std::mutex mtx;
	std::condition_variable cv;
	i32 currentWorker = 0;

	TaskFlow flow;
	flow.parallel_for(
		0, i32(workerPool->num_workers()), 1, [&mtx, &cv, &currentWorker, workerPoolSize](i32 i) {
			ScopedZone("Setup thread", 459bd1);

			std::unique_lock<std::mutex> lck(mtx);
			++currentWorker;
			cv.notify_all();
			while (workerPoolSize != currentWorker)
			{
				cv.wait(lck);
			}
			{
				ScopedZone("Naming thread", 459bd1);
				// Name each worker thread in the debugger
				tracy::SetThreadName(CString::Format("Worker {}", i + 1).c_str());
			}
		});
	auto future = RunFlow(flow);
	cv.notify_all();
	future.wait();
}

TaskSystem& TaskSystem::Get()
{
	return Context::Get()->GetTasks();
}