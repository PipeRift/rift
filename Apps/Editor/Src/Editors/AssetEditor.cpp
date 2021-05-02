// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/AssetEditor.h"

#include "Editors/ProjectEditor.h"

#include <imgui_internal.h>


namespace Rift
{
	const Name AssetEditor::rightNode{"rightNode"};
	const Name AssetEditor::centralNode{"centralNode"};


	AssetEditor::AssetEditor() : Super()
	{
		layout.OnBuild([](auto& builder) {
			// =================================== //
			//                           |         //
			//                           |         //
			//          Central          |  Right  //
			//          (Graph)          |(Details)//
			//                           |         //
			//                           |         //
			// =================================== //
			builder.Split(builder.GetRootNode(), ImGuiDir_Right, 0.2f, rightNode, centralNode);

			builder.GetNodeLocalFlags(rightNode) |= ImGuiDockNodeFlags_AutoHideTabBar;
			builder.GetNodeLocalFlags(centralNode) |= ImGuiDockNodeFlags_CentralNode;
		});
	}

	void AssetEditor::Draw()
	{
		auto owner = GetOwner<ProjectEditor>();
		assert(owner);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		bool isOpen       = true;
		String windowName = asset.GetFilename();
		Strings::FormatTo(windowName, TX("###{}"), asset.GetStrPath());

		if (pendingFocus)
		{
			ImGui::SetWindowFocus(windowName.c_str());
			pendingFocus = false;
		}

		owner->layout.BindNextWindowToNode(ProjectEditor::centralNode);
		if (ImGui::Begin(windowName.c_str(), &isOpen))
		{
			ImGui::PopStyleVar(3);

			CreateDockspace(windowName.c_str());
			layout.Tick(dockspaceID);

			nodeGraph.Draw(asset.GetStrPath(), layout);
		}
		else
		{
			ImGui::PopStyleVar(3);
		}
		ImGui::End();

		if (!isOpen)
		{
			owner->CloseType(asset);
		}
	}

	void AssetEditor::CreateDockspace(const char* id)
	{
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		dockspaceID = ImGui::GetID(id);
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	}
}    // namespace Rift
