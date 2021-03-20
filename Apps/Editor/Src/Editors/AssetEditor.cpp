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
		owner->layout.BindNextWindowToNode(ProjectEditor::centralNode);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		bool isOpen = true;
		ImGui::PushID(asset.GetStrPath().data());
		if (ImGui::Begin(asset.GetFilename().c_str(), &isOpen))
		{
			ImGui::PopStyleVar(3);

			CreateDockspace();
			layout.Tick(dockspaceID);

			nodeGraph.Draw(layout);
		}
		else
		{
			ImGui::PopStyleVar(3);
		}
		ImGui::End();
		ImGui::PopID();

		if (!isOpen)
		{
			owner->CloseType(asset);
		}
	}

	void AssetEditor::CreateDockspace()
	{
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		dockspaceID = ImGui::GetID(asset.GetStrPath().data());
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	}
}    // namespace Rift
