// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Pipe/Reflect/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift::Editor
{
	struct CModuleEditor : public p::Struct
	{
		STRUCT(CModuleEditor, p::Struct, p::Struct_NotSerialized)

		bool pendingFocus = false;
	};
}    // namespace rift::Editor
