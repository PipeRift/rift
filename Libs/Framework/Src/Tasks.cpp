// Copyright 2015-2022 Piperift - All rights reserved

#include "Tasks.h"

#include "RiftContext.h"

#include <PCore/Profiler.h>
#include <PCore/String.h>
#include <PMath/Math.h>

#include <chrono>
#include <common/TracySystem.hpp>
#include <condition_variable>
#include <mutex>



namespace rift
{
	TaskSystem::TaskSystem()
	    : mainPool{p::math::Min(1u, std::thread::hardware_concurrency())}
	    , workerPool{
	          p::math::Max(1u, std::thread::hardware_concurrency() - i32(mainPool.num_workers()))}
	{
		// Name main thread
		tracy::SetThreadName("Main");

		// Go over all worker threads naming them and making sure no thread is repeated
		TaskFlow flow;
		std::mutex mtx;
		std::condition_variable cv;
		i32 currentWorker        = 0;
		const i32 workerPoolSize = i32(workerPool.num_workers());
		flow.for_each_index(0, workerPoolSize, 1,
		    [&mtx, &cv, &currentWorker, workerPoolSize](i32 i) {
			std::unique_lock<std::mutex> lck(mtx);
			++currentWorker;
			cv.notify_all();
			while (workerPoolSize != currentWorker)
			{
				cv.wait(lck);
			}
			{
				// Name each worker thread in the debugger
				tracy::SetThreadName(Strings::Format("Worker {}", i + 1).c_str());
			}
		});
		auto future = Run(flow, TaskPool::Workers);
		cv.notify_all();
		future.wait();
	}

	ThreadPool& TaskSystem::GetPool(TaskPool pool)
	{
		switch (pool)
		{
			case TaskPool::Workers: return workerPool;
			case TaskPool::Main: return mainPool;
		}
		CheckMsg(false, "Requested an invalid pool");
		static ThreadPool invalidPool{0};
		return invalidPool;
	}

	TaskSystem& TaskSystem::Get()
	{
		return p::GetContext<RiftContext>()->GetTasks();
	}
}    // namespace rift
