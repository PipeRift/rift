// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/ProjectEditor.h"
#include "Editors/TypeAssetEditor.h"

#include <imgui_internal.h>



namespace Rift
{
	const Name TypeAssetEditor::rightNode{"rightNode"};
	const Name TypeAssetEditor::centralNode{"centralNode"};


	TypeAssetEditor::TypeAssetEditor() : Super()
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

	void TypeAssetEditor::Draw()
	{
		auto owner = GetOwner<ProjectEditor>();
		assert(owner);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		bool isOpen       = true;
		String windowName = asset.GetFilename();
		Strings::FormatTo(windowName, TX("###{}"), GetWindowId());

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

			nodeGraph.Draw(layout);
			properties.Draw(layout);
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

	void TypeAssetEditor::CreateDockspace(const char* id)
	{
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		dockspaceID = ImGui::GetID(id);
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	}
}    // namespace Rift
