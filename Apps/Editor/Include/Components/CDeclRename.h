// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypePropertiesPanel.h"
#include "DockSpaceLayout.h"
#include "NodeGraph/NodeGraphPanel.h"

#include <Types/Struct.h>
#include <UI/UI.h>


namespace Rift
{
	struct CDeclRename : public Struct
	{
		STRUCT(CDeclRename, Struct, Struct_NotSerialized)

		// Renaming uses this buffer to temporarely store the name being edited
		String buffer;
	};
}    // namespace Rift
