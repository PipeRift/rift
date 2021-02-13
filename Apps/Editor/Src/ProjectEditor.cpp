// Copyright 2015-2021 Piperift - All rights reserved

#include "ProjectEditor.h"

#include <imgui_internal.h>


void ProjectEditor::SetProject(Path path)
{
	if (project && project->GetPath() == path)
	{
		return;	   // Same project, ignore call
	}

	assetEditors.Empty();	 // Close previously opened editors
	project = Create<Project>();
	project->Init(path);
}

void ProjectEditor::OpenType(TAssetPtr<TypeAsset> asset)
{
	if (assetEditors.Contains([asset](const auto& editor) {
			return editor->GetAsset() == asset;
		}))
	{
		// Cant open the same asset twice. We just focus it
		// TODO: Focus on the already oppenned asset editor
		return;
	}

	assetEditors.Add(Create<AssetEditor>());
	assetEditors.Last()->SetAsset(asset);
}

void ProjectEditor::Draw()
{
	CreateDockspace();
	static bool doOnce = true;
	if (doOnce || ImGui::DockBuilderGetNode(dockspaceID) == nullptr)
	{
		doOnce = false;
		ResetLayout();	  // Initialize default layout if layout was not set
	}

	fileExplorer.Draw();
	for (const auto& editor : assetEditors)
	{
		if (editor.IsValid())
		{
			editor->Draw();
		}
	}
}


void ProjectEditor::ResetLayout()
{
	ImVec2 dockspaceSize = ImGui::GetMainViewport()->Size;
	ImGui::DockBuilderRemoveNode(dockspaceID);							// Clear out existing layout
	ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);	// Add empty node
	ImGui::DockBuilderSetNodeSize(dockspaceID, dockspaceSize);

	// ============================================== //
	//          |                                     //
	//          |                                     //
	//          |                                     //
	//   File   |                Files                //
	// Explorer |                                     //
	//          |                                     //
	//          |                                     //
	//          |                                     //
	//          |                                     //
	// ============================================== //

	ImGui::DockBuilderSplitNode(
		dockspaceID, ImGuiDir_Left, 0.2f, &fileExplorerDockID, &filesDockID);

	ImGui::DockBuilderGetNode(fileExplorerDockID)->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::DockBuilderGetNode(filesDockID)->LocalFlags |= ImGuiDockNodeFlags_CentralNode;

	ImGui::DockBuilderFinish(dockspaceID);
}

void ProjectEditor::CreateDockspace()
{
	ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

	const auto& viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags hostWindowFlags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (dockingFlags & ImGuiDockNodeFlags_PassthruCentralNode)
	{
		hostWindowFlags |= ImGuiWindowFlags_NoBackground;
	}

	char label[32];
	ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(label, nullptr, hostWindowFlags);
	ImGui::PopStyleVar(3);

	dockspaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	ImGui::End();
}
