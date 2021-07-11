// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"
#include "Editors/BaseEditor.h"
#include "Editors/Projects/FileExplorerPanel.h"
#include "Editors/TypeAssetEditor.h"
#include "Tools/ASTDebugger.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>
#include <Files/FileWatcher.h>
#include <Module.h>
#include <UI/UI.h>


namespace Rift
{
	class ProjectEditor : public BaseEditor
	{
		CLASS(ProjectEditor, BaseEditor)

	public:
		static const Name leftNode;
		static const Name centralNode;

		ImGuiID dockspaceID = 0;

		String currentProjectPath;
		TArray<TOwnPtr<TypeAssetEditor>> TypeAssetEditors;
		TArray<TAssetPtr<TypeAsset>> pendingTypesToClose;

		FileExplorerPanel fileExplorer{*this};
		DockSpaceLayout layout;
		Rift::ASTDebugger astDebugger;

		FileWatcher fileWatcher;

	protected:
		bool bSkipFrameAfterMenu = false;


	public:
		ProjectEditor();
		void BeforeDestroy() override;
		void OpenType(TAssetPtr<TypeAsset> asset);
		void CloseType(TAssetPtr<TypeAsset> asset);

		void Draw();

	protected:
		void CreateDockspace();

		void DrawMenuBar(AST::Tree& ast);

		void OnProjectChanged();
	};
}    // namespace Rift
