// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <EASTL/memory.h>

#include <taskflow/taskflow.hpp>


EA_DISABLE_VC_WARNING(4267)

using TaskFlow = tf::Taskflow;
using Task = tf::Task;
using Flow = tf::FlowBuilder;
using SubFlow = tf::SubflowBuilder;

using TaskLambda = eastl::function<void()>;
using SubTaskLambda = eastl::function<void(tf::SubflowBuilder&)>;


struct TaskSystem
{
	using ThreadPool = tf::Executor;

private:
	// Render & Game thread
	std::shared_ptr<ThreadPool> gamePool;
	// Worker threads
	std::shared_ptr<ThreadPool> workerPool;

public:
	TaskSystem();

	// Runs a flow in Workers thread pool
	std::future<void> RunFlow(TaskFlow& flow) const
	{
		return workerPool->run(flow);
	}

	// Creates a flow in Game thread pool
	std::future<void> RunGameFlow(TaskFlow& flow) const
	{
		return gamePool->run(flow);
	}

	u32 GetNumWorkerThreads() const
	{
		return (u32) workerPool->num_workers();
	}
};

EA_RESTORE_VC_WARNING()	   // warning: 4267