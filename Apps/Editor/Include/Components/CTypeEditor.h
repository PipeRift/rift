// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <CoreObject.h>
#include <UI/UI.h>


namespace Rift
{
	struct CTypeEditor : public Struct
	{
		STRUCT(CTypeEditor, Struct, Struct_Transient)

		static const Name rightNode;
		static const Name centralNode;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		bool pendingFocus = false;

		AST::Id selectedPropertyId      = AST::NoId;
		AST::Id pendingDeletePropertyId = AST::NoId;
	};

	inline const Name CTypeEditor::rightNode{"rightNode"};
	inline const Name CTypeEditor::centralNode{"centralNode"};
}    // namespace Rift
