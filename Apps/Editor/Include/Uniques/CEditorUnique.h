// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "DockSpaceLayout.h"
#include "Panels/FileExplorerPanel.h"
#include "Tools/ASTDebugger.h"

#include <CoreObject.h>
#include <Files/FileWatcher.h>
#include <Memory/OwnPtr.h>
#include <UI/UI.h>


namespace Rift
{
	struct CEditorUnique : public Struct
	{
		STRUCT(CEditorUnique, Struct)

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;
		static const Name leftNode;
		static const Name centralNode;

		String currentProjectPath;
		TArray<AST::Id> pendingTypesToClose;

		FileWatcher fileWatcher;
		FileExplorerPanel fileExplorer{};

		ASTDebugger astDebugger;

		bool skipFrameAfterMenu = false;
	};

	inline const Name CEditorUnique::leftNode{"leftNode"};
	inline const Name CEditorUnique::centralNode{"centralNode"};
}    // namespace Rift
