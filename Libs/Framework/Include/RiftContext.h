// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Plugin.h"
#include "View.h"

#include <Context.h>


namespace rift
{
	/**
	 * A solution represents an open project and all its build dependencies.
	 */
	// NOTE: Should this be removed?
	class RiftContext : public Context
	{
		CLASS(RiftContext, Context)

	protected:
		TArray<TOwnPtr<Plugin>> plugins;

		TArray<View> views;


	public:
		template<typename T>
		void AddPlugin()
		{
			plugins.Add(MakeOwned<T>());
		}

		template<typename T>
		void AddView(T view)
		{
			views.Add(Move(view));
		}
	};
}    // namespace rift
