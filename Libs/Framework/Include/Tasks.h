// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>

#include <memory>
#include <taskflow/taskflow.hpp>


namespace rift
{
	using TaskFlow = tf::Taskflow;
	using Task     = tf::Task;
	using Flow     = tf::FlowBuilder;

	using TaskLambda    = std::function<void()>;
	using SubTaskLambda = std::function<void(Flow&)>;
	using ThreadPool    = tf::Executor;

	enum class TaskPool : p::u8
	{
		Main,
		Workers
	};

	struct TaskSystem
	{
	private:
		// Main thread
		ThreadPool mainPool;
		// Worker threads
		ThreadPool workerPool;


	public:
		TaskSystem();

		ThreadPool& GetPool(TaskPool pool);

		// Runs a flow in Workers thread pool
		std::future<void> Run(TaskFlow& flow, TaskPool pool = TaskPool::Workers)
		{
			return GetPool(pool).run(flow);
		}

		// Creates a flow in Game thread pool
		std::future<void> RunMainFlow(TaskFlow& flow)
		{
			return mainPool.run(flow);
		}

		p::u32 GetNumWorkerThreads() const
		{
			return static_cast<p::u32>(workerPool.num_workers());
		}
	};
}    // namespace rift
