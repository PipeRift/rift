// Copyright 2015-2023 Piperift - All rights reserved

#include "DockSpaceLayout.h"

#include <imgui_internal.h>
#include <Pipe/Core/Log.h>
#include <UI/UI.h>


namespace rift::Editor
{
	const Tag DockSpaceLayout::rootNodeId{"root"};


	bool DockSpaceLayout::Builder::Split(Tag originNodeId, ImGuiDir direction, float splitRatio,
	    Tag newNodeID, Tag oppositeNewNodeID)
	{
		const ImGuiID originDockNodeId = layout.GetDockNodeId(originNodeId);
		if (originDockNodeId == 0)
		{
			p::Error("Error splitting node {}. It doesn't exist.", originNodeId.AsString());
			return false;
		}

		ImGuiID newID = 0, oppositeNewID = 0;
		ImGui::DockBuilderSplitNode(
		    originDockNodeId, direction, splitRatio, &newID, &oppositeNewID);

		layout.nameToDockNodeId.Insert(newNodeID, newID);
		layout.nameToDockNodeId.Insert(oppositeNewNodeID, oppositeNewID);
		return true;
	}

	ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeLocalFlags(Tag nodeId)
	{
		const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
		Check(dockNodeId > 0);
		return ImGui::DockBuilderGetNode(dockNodeId)->LocalFlags;
	}

	ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeSharedFlags(Tag nodeId)
	{
		const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
		Check(dockNodeId > 0);
		return ImGui::DockBuilderGetNode(dockNodeId)->SharedFlags;
	}


	void DockSpaceLayout::Tick(ImGuiID dockSpaceID)
	{
		// Keep root id always updated
		nameToDockNodeId.Insert(rootNodeId, dockSpaceID);

		if (bCurrentlyReseting)
		{
			bCurrentlyReseting = false;
		}

		// If layout was not ever set or we want to reset, reset it
		if (bWantsToReset || ImGui::DockBuilderGetNode(dockSpaceID) == nullptr)
		{
			Rebuild(dockSpaceID);
			bWantsToReset      = false;
			bCurrentlyReseting = true;
		}
	}

	void DockSpaceLayout::BindWindowToNode(StringView windowId, Tag nodeId)
	{
		ImGuiID dockId = GetDockNodeId(nodeId);
		if (dockId > 0)
		{
			ImGui::DockBuilderDockWindow(windowId.data(), dockId);
		}
	}
	void DockSpaceLayout::BindNextWindowToNode(Tag nodeId, ImGuiCond cond)
	{
		ImGuiID dockId = GetDockNodeId(nodeId);
		if (dockId > 0)
		{
			if (bCurrentlyReseting)
			{
				cond = ImGuiCond_Always;
			}
			ImGui::SetNextWindowDockID(dockId, cond);
		}
	}

	void DockSpaceLayout::Rebuild(ImGuiID dockSpaceID)
	{
		// Clear out existing layout
		ImGui::DockBuilderRemoveNode(dockSpaceID);

		// Add root node
		ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
		// Fill the dockspace
		ImGui::DockBuilderSetNodeSize(dockSpaceID, ImGui::GetMainViewport()->Size);

		if (onBuild)
		{
			Builder builder(*this);
			onBuild(builder);
		}
		ImGui::DockBuilderFinish(dockSpaceID);
	}
}    // namespace rift::Editor
