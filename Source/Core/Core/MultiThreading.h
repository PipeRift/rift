// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include <EASTL/memory.h>
#include <taskflow/taskflow.hpp>


EA_DISABLE_VC_WARNING(4267)

using TaskFlow = tf::Taskflow;
using Task     = tf::Task;
using Flow     = tf::FlowBuilder;
using SubFlow  = tf::SubflowBuilder;

using TaskLambda = eastl::function<void()>;
using SubTaskLambda = eastl::function<void(tf::SubflowBuilder&)>;


struct TaskSystem {
	using ThreadPool = TaskFlow::Executor;

private:
	// Render & Game thread
	std::shared_ptr<ThreadPool> gamePool;
	// Worker threads
	std::shared_ptr<ThreadPool> workerPool;

public:

	TaskSystem();

	// Creates a flow in Workers thread pool
	FORCEINLINE TaskFlow CreateFlow() const {
		return TaskFlow{ workerPool };
	}

	// Creates a flow in Game thread pool
	FORCEINLINE TaskFlow CreateMainFlow() const {
		return TaskFlow{ gamePool };
	}

	FORCEINLINE u32 GetNumWorkerThreads() const { return (u32)workerPool->num_workers(); }
};

EA_RESTORE_VC_WARNING() // warning: 4267
