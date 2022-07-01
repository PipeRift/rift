// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Pipe/Reflect/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift
{
	struct CTypeEditor : public p::Struct
	{
		STRUCT(CTypeEditor, p::Struct, p::Struct_NotSerialized)

		static const Name rightTopNode;
		static const Name rightBottomNode;
		static const Name centralNode;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		bool pendingFocus = false;
		bool showGraph    = true;
		bool showElements = true;
		bool showDetails  = true;

		AST::Id selectedPropertyId      = AST::NoId;
		AST::Id pendingDeletePropertyId = AST::NoId;

		Nodes::EditorContext nodesEditor;
		ImGuiTextFilter elementsFilter;
	};

	inline const Name CTypeEditor::rightTopNode{"rightTopNode"};
	inline const Name CTypeEditor::rightBottomNode{"rightBottomNode"};
	inline const Name CTypeEditor::centralNode{"centralNode"};
}    // namespace rift
