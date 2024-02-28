// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <AST/Id.h>
#include <Pipe/Reflect/Struct.h>
#include <UI/UI.h>
#include <Utils/NodesInternal.h>


namespace rift::editor
{
	struct CTypeEditor : public p::Struct
	{
		P_STRUCT(CTypeEditor, p::Struct, p::Struct_NotSerialized)

		static const Tag rightTopNode;
		static const Tag rightBottomNode;
		static const Tag centralNode;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		bool pendingFocus = false;
		bool showGraph    = true;
		bool showElements = true;
		bool showDetails  = true;

		ast::Id selectedPropertyId      = ast::NoId;
		ast::Id pendingDeletePropertyId = ast::NoId;

		Nodes::EditorContext nodesEditor;
		ImGuiTextFilter elementsFilter;
	};

	inline const Tag CTypeEditor::rightTopNode{"rightTopNode"};
	inline const Tag CTypeEditor::rightBottomNode{"rightBottomNode"};
	inline const Tag CTypeEditor::centralNode{"centralNode"};
}    // namespace rift::editor
