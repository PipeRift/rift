// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "DockSpaceLayout.h"
#include "Panels/FileExplorerPanel.h"
#include "Tools/ASTDebugger.h"
#include "Tools/GraphPlayground.h"
#include "Tools/ReflectionDebugger.h"

#include <Files/FileWatcher.h>
#include <Memory/OwnPtr.h>
#include <Reflection/Struct.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace Pipe::Core;
	using namespace Pipe::Files;


	struct SEditor : public Struct
	{
		STRUCT(SEditor, Struct)

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;
		static const Name leftNode;
		static const Name centralNode;

		String currentProjectPath;
		TArray<AST::Id> pendingTypesToClose;

		FileWatcher fileWatcher;
		FileExplorerPanel fileExplorer{};

		ReflectionDebugger reflectionDebugger;
		ASTDebugger astDebugger;
		GraphPlayground graphPlayground;

		bool skipFrameAfterMenu = false;
	};

	inline const Name SEditor::leftNode{"leftNode"};
	inline const Name SEditor::centralNode{"centralNode"};
}    // namespace Rift
