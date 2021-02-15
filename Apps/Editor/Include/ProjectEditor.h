// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AssetEditor.h"
#include "FileExplorerPanel.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>
#include <CoreObject.h>
#include <Project.h>
#include <imgui.h>


using namespace Rift;


class ProjectEditor : public Rift::Object
{
	CLASS(ProjectEditor, Rift::Object)

public:
	ObjectPtr<Project> project;

	ImGuiID dockspaceID        = 0;
	ImGuiID fileExplorerDockID = 0;
	ImGuiID filesDockID        = 0;
	TArray<ObjectPtr<AssetEditor>> assetEditors;

	FileExplorerPanel fileExplorer{*this};
	bool bWantsToResetLayout = false;

protected:
	bool bSkipFrameAfterMenu = false;


public:
	ProjectEditor() = default;

	void BeforeDestroy() override;
	void SetProject(Path path);
	void OpenType(TAssetPtr<TypeAsset> asset);

	void Draw();

	void ResetLayout();

	bool HasProject()
	{
		return project && project->IsValid();
	}

protected:
	void CreateDockspace();

	void DrawMenuBar();
};
