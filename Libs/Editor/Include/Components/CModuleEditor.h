// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift::editor
{
	struct CModuleEditor
	{
		P_STRUCT(CModuleEditor, p::TF_NotSerialized)

		bool pendingFocus = false;
	};
}    // namespace rift::editor
