// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/ProjectEditor.h"
#include "Window.h"

#include <Files/FileDialog.h>
#include <imgui_internal.h>


const Name ProjectEditor::leftNode{"leftNode"};
const Name ProjectEditor::centralNode{"centralNode"};

ProjectEditor::ProjectEditor() : Super()
{
	layout.OnBuild([](auto& builder) {
		// ==================================== //
		//          |                           //
		//          |                           //
		//   Left   |          Central          //
		//(Explorer)|          (Types)          //
		//          |                           //
		//          |                           //
		// ==================================== //
		builder.Split(builder.GetRootNode(), ImGuiDir_Left, 0.2f, leftNode, centralNode);

		builder.GetNodeLocalFlags(leftNode) |= ImGuiDockNodeFlags_AutoHideTabBar;
		builder.GetNodeLocalFlags(centralNode) |= ImGuiDockNodeFlags_CentralNode;
	});
}

void ProjectEditor::BeforeDestroy()
{
	Super::BeforeDestroy();

	// Set config path to project folder and save or load manually=
	Window::Get().SetUIConfigFile({});
}

void ProjectEditor::SetProject(Path path)
{
	if (project && project->GetPath() == path)
	{
		return;    // Same project, ignore call
	}

	// TODO: Clear project if no path

	assetEditors.Empty();    // Close previously opened editors
	project = Create<Project>();
	project->Init(path);

	if (HasProject())
	{
		// Set config path to project folder and save or load manually=
		Window::Get().SetUIConfigFile(path / "Saved" / "ui.ini");

		// Just for testing
		assetEditors.Add(Create<AssetEditor>(Self()));
	}
}

void ProjectEditor::OpenType(TAssetPtr<TypeAsset> asset)
{
	if (assetEditors.Contains([asset](const auto& editor) {
		    return editor->GetAsset() == asset;
	    }))
	{
		// Cant open the same asset twice. We just focus it
		// TODO: Focus on the already oppened asset editor
		return;
	}

	assetEditors.Add(Create<AssetEditor>(Self()));
	assetEditors.Last()->SetAsset(asset);
}

void ProjectEditor::Draw()
{
	String projectPath = FileSystem::ToString(project->GetPath());
	ImGui::PushID(Hash<String>()(projectPath));

	DrawMenuBar();

	if (bSkipFrameAfterMenu)    // We could have closed the project
	{
		bSkipFrameAfterMenu = false;
		ImGui::PopID();
		return;
	}

	CreateDockspace();
	layout.Tick(dockspaceID);

	fileExplorer.Draw();
	for (const auto& editor : assetEditors)
	{
		if (editor)
		{
			editor->Draw();
		}
	}

	ImGui::PopID();
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

void ProjectEditor::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project"))
			{
				Path folder =
				    Dialogs::SelectFolder("Select project folder", FileSystem::GetCurrent());
				if (Window::Get().GetRootEditor().OpenProject(folder))
				{
					bSkipFrameAfterMenu = true;
				}
			}
			if (ImGui::MenuItem("Close current"))
			{
				Window::Get().GetRootEditor().CloseProject();
				bSkipFrameAfterMenu = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open File")) {}
			if (ImGui::MenuItem("Save File", "CTRL+S")) {}
			if (ImGui::MenuItem("Save All", "CTRL+SHFT+S")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}    // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Layout"))
		{
			if (ImGui::MenuItem("Reset layout"))
			{
				layout.Reset();
				for (const auto& editor : assetEditors)
				{
					if (editor)
					{
						editor->GetLayout().Reset();
					}
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
