// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Assets/TypePropertiesPanel.h"
#include "DockSpaceLayout.h"
#include "NodeGraph/NodeGraphPanel.h"

#include <Reflection/Struct.h>
#include <UI/UI.h>


namespace rift
{
	struct CDeclRename : public p::Struct
	{
		STRUCT(CDeclRename, p::Struct, p::Struct_NotSerialized)

		// Renaming uses this buffer to temporarely store the name being edited
		String buffer;
	};
}    // namespace rift
