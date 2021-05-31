// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Platform/Platform.h>
#include <Strings/StringView.h>


namespace Rift
{
	class TypeAssetEditor;

	class NodeGraphPanel
	{
		TypeAssetEditor& editor;

	public:
		NodeGraphPanel(TypeAssetEditor& editor);
		void Draw(struct DockSpaceLayout& layout);
	};
}    // namespace Rift
