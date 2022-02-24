// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Types.h>
#include <Types/Struct.h>
#include <UI/NodesInternal.h>
#include <UI/UI.h>


namespace Rift
{
	struct CModuleEditor : public Struct
	{
		STRUCT(CModuleEditor, Struct, Struct_NotSerialized)

		bool pendingFocus = false;
	};
}    // namespace Rift
