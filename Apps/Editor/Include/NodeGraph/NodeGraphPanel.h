// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>
#include <Strings/StringView.h>


namespace Rift
{
	class NodeGraphPanel
	{
	public:
		NodeGraphPanel();
		~NodeGraphPanel();
		void Draw(StringView baseId, struct DockSpaceLayout& layout);
	};
}    // namespace Rift
