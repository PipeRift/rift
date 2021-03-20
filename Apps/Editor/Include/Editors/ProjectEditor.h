// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"
#include "Editors/AssetEditor.h"
#include "Editors/BaseEditor.h"
#include "Editors/Projects/FileExplorerPanel.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>
#include <Project.h>
#include <UI/UI.h>


using namespace Rift;


class ProjectEditor : public BaseEditor
{
	CLASS(ProjectEditor, BaseEditor)

public:
	static const Name leftNode;
	static const Name centralNode;

	ObjectPtr<Project> project;
	ImGuiID dockspaceID = 0;

	TArray<ObjectPtr<AssetEditor>> assetEditors;
	TArray<TAssetPtr<TypeAsset>> pendingTypesToClose;

	FileExplorerPanel fileExplorer{*this};
	DockSpaceLayout layout;

protected:
	bool bSkipFrameAfterMenu = false;


public:
	ProjectEditor();
	void BeforeDestroy() override;
	void SetProject(Path path);
	void OpenType(TAssetPtr<TypeAsset> asset);
	void CloseType(TAssetPtr<TypeAsset> asset);

	void Draw();

	bool HasProject()
	{
		return project && project->IsValid();
	}

protected:
	void CreateDockspace();

	void DrawMenuBar();
};
