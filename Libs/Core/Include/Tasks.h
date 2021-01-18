// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <memory>
#include <taskflow/taskflow.hpp>


// EA_DISABLE_VC_WARNING(4267);


namespace Rift
{
	using TaskFlow = tf::Taskflow;
	using Task = tf::Task;
	using Flow = tf::FlowBuilder;
	using SubFlow = tf::SubflowBuilder;

	using TaskLambda = std::function<void()>;
	using SubTaskLambda = std::function<void(tf::SubflowBuilder&)>;


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

		static TaskSystem& Get();
	};
}	 // namespace Rift
