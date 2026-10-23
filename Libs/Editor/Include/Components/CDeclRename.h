// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Assets/TypePropertiesPanel.h"
#include "DockSpaceLayout.h"
#include "NodeGraph/NodeGraphPanel.h"

#include <UI/UI.h>


namespace rift::editor
{
	struct CDeclRename
	{
		P_STRUCT(CDeclRename, p::TF_NotSerialized)

		// Renaming uses this buffer to temporarely store the name being edited
		p::String buffer;
	};
}    // namespace rift::editor
