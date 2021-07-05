// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"
#include "Editors/ProjectEditor.h"

#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <Files/FileDialog.h>
#include <Framework/Paths.h>
#include <Profiler.h>
#include <RiftContext.h>
#include <UI/UI.h>
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

		String projectPath = Paths::ToString(Modules::GetProjectPath(Editor::GetAST()));
		if (projectPath != currentProjectPath)
		{
			currentProjectPath = projectPath;
			OnProjectChanged();
		}

		UI::PushID(Hash<Name>()(projectPath));

		DrawMenuBar();

		if (bSkipFrameAfterMenu)    // We could have closed the project
		{
			bSkipFrameAfterMenu = false;
			UI::PopID();
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

		astDebugger.Draw(Editor::GetAST());
		UI::PopID();
	}

	void ProjectEditor::CreateDockspace()
	{
		ZoneScoped;
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		const auto& viewport = UI::GetMainViewport();

		UI::SetNextWindowPos(viewport->WorkPos);
		UI::SetNextWindowSize(viewport->WorkSize);
		UI::SetNextWindowViewport(viewport->ID);

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

		UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		UI::Begin(label, nullptr, hostWindowFlags);
		UI::PopStyleVar(3);

		dockspaceID = UI::GetID("DockSpace");
		UI::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
		UI::End();
	}

	void ProjectEditor::DrawMenuBar()
	{
		ZoneScoped;

		auto context = GetContext<RiftContext>();
		if (UI::BeginMainMenuBar())
		{
			if (UI::BeginMenu("File"))
			{
				if (UI::MenuItem("Open Project"))
				{
					const Path folder =
					    Dialogs::SelectFolder("Select project folder", Paths::GetCurrent());
					if (Modules::OpenProject(Editor::GetAST(), folder))
					{
						bSkipFrameAfterMenu = true;
					}
				}
				if (UI::MenuItem("Close current"))
				{
					Modules::CloseProject(Editor::GetAST());
					bSkipFrameAfterMenu = true;
				}
				UI::Separator();
				if (UI::MenuItem("Open File")) {}
				if (UI::MenuItem("Save File", "CTRL+S")) {}
				if (UI::MenuItem("Save All", "CTRL+SHFT+S")) {}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Build"))
			{
				if (UI::MenuItem("Build current"))
				{
					AST::Tree compileAST;
					compileAST.CopyFrom(Editor::GetAST());
					Rift::Compiler::Config config;
					Rift::Compiler::Build(compileAST, config, Rift::Compiler::EBackend::Cpp);
				}
				if (UI::MenuItem("Build all"))
				{
					AST::Tree compileAST;
					compileAST.CopyFrom(Editor::GetAST());
					Rift::Compiler::Config config;
					Rift::Compiler::Build(compileAST, config, Rift::Compiler::EBackend::Cpp);
				}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Edit"))
			{
				if (UI::MenuItem("Undo", "CTRL+Z")) {}
				if (UI::MenuItem("Redo", "CTRL+Y", false, false)) {}    // Disabled item
				UI::Separator();
				if (UI::MenuItem("Cut", "CTRL+X")) {}
				if (UI::MenuItem("Copy", "CTRL+C")) {}
				if (UI::MenuItem("Paste", "CTRL+V")) {}
				UI::EndMenu();
			}

			if (UI::BeginMenu("Views"))
			{
				UI::MenuItem("Syntax Tree", nullptr, &astDebugger.open);
				UI::EndMenu();
			}

			if (UI::BeginMenu("Layout"))
			{
				if (UI::MenuItem("Reset layout"))
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
				UI::EndMenu();
			}

			UI::EndMainMenuBar();
		}
	}


	void ProjectEditor::OnProjectChanged()
	{
		TypeAssetEditors.Empty();

		if (!currentProjectPath.empty())
		{
			// Set config path to project folder and save or load manually
			Editor::Get().SetUIConfigFile(
			    Paths::FromString(currentProjectPath) / "Saved" / "ui.ini");
		}
		else
		{
			// Set default config path
			Editor::Get().SetUIConfigFile({});
		}

		fileWatcher.AddExtension(Paths::codeExtension);
		fileWatcher.AddPath(currentProjectPath);
	}
}    // namespace Rift
