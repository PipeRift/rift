// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/ProjectEditor.h"

#include "Editor.h"

#include <Compiler/Compiler.h>
#include <Files/FileDialog.h>
#include <Profiler.h>
#include <RiftContext.h>
#include <imgui_internal.h>


namespace Rift
{
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
		Editor::Get().SetUIConfigFile({});
	}

	void ProjectEditor::OpenType(TAssetPtr<TypeAsset> asset)
	{
		auto* existingEditor = TypeAssetEditors.Find([asset](const auto& editor) {
			return editor->GetAsset() == asset;
		});
		if (existingEditor)
		{
			// Cant open the same asset twice. We just focus it
			(*existingEditor)->MarkPendingFocus();
			return;
		}

		if (!asset.Load())
		{
			Log::Error("Couldn't open type editor. File can't be loaded.");
			return;
		}

		TypeAssetEditors.Add(Create<TypeAssetEditor>(Self()));
		TypeAssetEditors.Last()->SetAsset(asset);
	}

	void ProjectEditor::CloseType(TAssetPtr<TypeAsset> asset)
	{
		pendingTypesToClose.Add(asset);
	}

	void ProjectEditor::Draw()
	{
		ZoneScoped;

		TPtr<Project> project = RiftContext::GetProject();
		if (project != currentProject)
		{
			OnProjectChanged(project);
		}

		String projectPath = Paths::ToString(project->GetPath());
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


		// Close editors if needed
		TypeAssetEditors.RemoveIf([this](const auto& item) {
			return !item || pendingTypesToClose.Contains(item->GetAsset());
		});
		pendingTypesToClose.Empty();

		for (const auto& editor : TypeAssetEditors)
		{
			editor->Draw();
		}

		astDebugger.Draw(*RiftContext::GetAST());
		ImGui::PopID();
	}

	void ProjectEditor::CreateDockspace()
	{
		ZoneScoped;
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
		ZoneScoped;

		auto context = GetContext<RiftContext>();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project"))
				{
					Path folder =
					    Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());
					if (context->OpenProject(folder))
					{
						bSkipFrameAfterMenu = true;
					}
				}
				if (ImGui::MenuItem("Close current"))
				{
					context->CloseProject();
					bSkipFrameAfterMenu = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Open File")) {}
				if (ImGui::MenuItem("Save File", "CTRL+S")) {}
				if (ImGui::MenuItem("Save All", "CTRL+SHFT+S")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Build"))
			{
				if (ImGui::MenuItem("Build current"))
				{
					Rift::Compiler::Config config;
					Rift::Compiler::Build(
					    context->GetRootProject(), config, Rift::Compiler::EBackend::C);
				}
				if (ImGui::MenuItem("Build all"))
				{
					Rift::Compiler::Config config;
					Rift::Compiler::Build(
					    context->GetRootProject(), config, Rift::Compiler::EBackend::C);
				}
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
					for (const auto& editor : TypeAssetEditors)
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


	void ProjectEditor::OnProjectChanged(TPtr<Project> newProject)
	{
		currentProject = newProject;
		TypeAssetEditors.Empty();

		if (currentProject)
		{
			// Set config path to project folder and save or load manually
			Editor::Get().SetUIConfigFile(currentProject->GetPath() / "Saved" / "ui.ini");
		}
		else
		{
			// Set default config path
			Editor::Get().SetUIConfigFile({});
		}
	}
}    // namespace Rift
