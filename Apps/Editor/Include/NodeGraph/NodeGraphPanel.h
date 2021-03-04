// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>


namespace Rift
{
	class NodeGraphPanel
	{
	public:
		NodeGraphPanel();
		~NodeGraphPanel();
		void Draw(struct DockSpaceLayout& layout);
	};
}    // namespace Rift
