// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/TypeAssetEditor.h"

#include "Editors/ProjectEditor.h"
#include "RiftContext.h"

#include <AST/Utils/DeclarationUtils.h>
#include <UI/UI.h>
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

		UI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		UI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
		UI::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		bool isOpen       = true;
		String windowName = asset.GetFilename();
		Strings::FormatTo(windowName, TX("###{}"), GetWindowId());

		if (pendingFocus)
		{
			UI::SetWindowFocus(windowName.c_str());
			pendingFocus = false;
		}

		owner->layout.BindNextWindowToNode(ProjectEditor::centralNode);
		if (UI::Begin(windowName.c_str(), &isOpen))
		{
			UI::PopStyleVar(3);

			CreateDockspace(windowName.c_str());
			layout.Tick(dockspaceID);

			if (Util::IsStructDecl(*RiftContext::GetAST(), GetNode()))
			{
				nodeGraph.Draw(layout);
			}
			properties.Draw(layout);
		}
		else
		{
			UI::PopStyleVar(3);
		}
		UI::End();

		if (!isOpen)
		{
			owner->CloseType(asset);
		}
	}

	void TypeAssetEditor::CreateDockspace(const char* id)
	{
		ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None;

		dockspaceID = UI::GetID(id);
		UI::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockingFlags, nullptr);
	}
}    // namespace Rift
