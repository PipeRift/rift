// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

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
	};
}    // namespace Rift
