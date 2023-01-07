// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "DockSpaceLayout.h"
#include "Panels/FileExplorerPanel.h"
#include "Tools/ASTDebugger.h"
#include "Tools/GraphPlayground.h"
#include "Tools/MemoryDebugger.h"
#include "Tools/ReflectionDebugger.h"

#include <Pipe/Files/FileWatcher.h>
#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/Struct.h>
#include <UI/UI.h>


namespace rift::Editor
{
	using namespace p::core;
	using namespace p::files;


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
		MemoryDebugger memoryDebugger;
		GraphPlayground graphPlayground;

		bool skipFrameAfterMenu = false;
	};

	inline const Name SEditor::leftNode{"leftNode"};
	inline const Name SEditor::centralNode{"centralNode"};
}    // namespace rift::Editor
