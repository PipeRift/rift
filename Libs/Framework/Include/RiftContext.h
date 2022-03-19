// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Plugin.h"

#include <Context.h>


namespace Rift
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


	public:
		template<typename T>
		void AddPlugin()
		{
			plugins.Add(MakeOwned<T>());
		}
	};
}    // namespace Rift
