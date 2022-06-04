// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Reflection/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace Rift
{
	struct CTypeEditor : public Pipe::Struct
	{
		STRUCT(CTypeEditor, Pipe::Struct, Pipe::Struct_NotSerialized)

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
}    // namespace Rift
