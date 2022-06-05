// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Reflection/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift
{
	struct CModuleEditor : public pipe::Struct
	{
		STRUCT(CModuleEditor, pipe::Struct, pipe::Struct_NotSerialized)

		bool pendingFocus = false;
	};
}    // namespace rift
