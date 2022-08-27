// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Plugin.h"
#include "Tasks.h"
#include "View.h"

#include <Pipe/Reflect/Class.h>


namespace rift
{
	class Rift : public Class
	{
		CLASS(Rift, Class)

	protected:
		TArray<TOwnPtr<Plugin>> plugins;
		TArray<View> views;
		TaskSystem tasks;


	public:
		template<typename T>
		void AddPlugin()
		{
			// TODO: Ensure unique
			plugins.Add(MakeOwned<T>());
		}

		template<typename T>
		void AddView(T view)
		{
			views.Add(Move(view));
		}

		TaskSystem& GetTasks()
		{
			return tasks;
		}
	};
}    // namespace rift
