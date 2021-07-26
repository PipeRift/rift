// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"

#include <CoreObject.h>
#include <UI/UI.h>


namespace Rift
{
	struct CTypeEditor : public Struct
	{
		STRUCT(CTypeEditor, Struct, Transient)

		static const Name rightNode;
		static const Name centralNode;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		bool pendingFocus = false;
	};

	inline const Name CTypeEditor::rightNode{"rightNode"};
	inline const Name CTypeEditor::centralNode{"centralNode"};
}    // namespace Rift
