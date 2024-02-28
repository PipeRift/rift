// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Assets/TypePropertiesPanel.h"
#include "DockSpaceLayout.h"
#include "NodeGraph/NodeGraphPanel.h"

#include <Pipe/Reflect/Struct.h>
#include <UI/UI.h>


namespace rift::editor
{
	struct CDeclRename : public p::Struct
	{
		P_STRUCT(CDeclRename, p::Struct, p::Struct_NotSerialized)

		// Renaming uses this buffer to temporarely store the name being edited
		p::String buffer;
	};
}    // namespace rift::editor
