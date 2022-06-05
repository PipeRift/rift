// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Reflection/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift
{
	struct CTypeEditor : public pipe::Struct
	{
		STRUCT(CTypeEditor, pipe::Struct, pipe::Struct_NotSerialized)

		static const Name rightNode;
		static const Name centralNode;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		bool pendingFocus = false;

		AST::Id selectedPropertyId      = AST::NoId;
		AST::Id pendingDeletePropertyId = AST::NoId;

		Nodes::EditorContext nodesEditor;
	};

	inline const Name CTypeEditor::rightNode{"rightNode"};
	inline const Name CTypeEditor::centralNode{"centralNode"};
}    // namespace rift
