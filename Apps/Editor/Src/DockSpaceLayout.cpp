// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"

#include <imgui_internal.h>
#include <Log.h>


namespace Rift
{
	const Name DockSpaceLayout::rootNodeId{"root"};


	bool DockSpaceLayout::Builder::Split(Name originNodeId, ImGuiDir direction, float splitRatio,
	    Name newNodeID, Name oppositeNewNodeID)
	{
		const ImGuiID originDockNodeId = layout.GetDockNodeId(originNodeId);
		if (originDockNodeId == 0)
		{
			Log::Error("Error splitting node {}. It doesn't exist.", originNodeId.ToString());
			return false;
		}

		ImGuiID newID = 0, oppositeNewID = 0;
		ImGui::DockBuilderSplitNode(
		    originDockNodeId, direction, splitRatio, &newID, &oppositeNewID);

		layout.nameToDockNodeId.Insert(newNodeID, newID);
		layout.nameToDockNodeId.Insert(oppositeNewNodeID, oppositeNewID);
		return true;
	}

	ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeLocalFlags(Name nodeId)
	{
		const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
		assert(dockNodeId > 0);
		return ImGui::DockBuilderGetNode(dockNodeId)->LocalFlags;
	}

	ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeSharedFlags(Name nodeId)
	{
		const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
		assert(dockNodeId > 0);
		return ImGui::DockBuilderGetNode(dockNodeId)->SharedFlags;
	}


	void DockSpaceLayout::Tick(ImGuiID dockSpaceID)
	{
		// Keep root id always updated
		nameToDockNodeId.Insert(rootNodeId, dockSpaceID);

		if (bCurrentlyReseting)
		{
			bCurrentlyReseting = false;
			bWantsToReset      = false;
		}

		// If layout was not ever set or we want to reset, reset it
		if (bWantsToReset || ImGui::DockBuilderGetNode(dockSpaceID) == nullptr)
		{
			DoReset(dockSpaceID);
			bCurrentlyReseting = true;
		}
	}

	void DockSpaceLayout::BindWindowToNode(StringView windowId, Name nodeId)
	{
		ImGuiID dockId = GetDockNodeId(nodeId);
		if (dockId > 0)
		{
			ImGui::DockBuilderDockWindow(windowId.data(), dockId);
		}
	}
	void DockSpaceLayout::BindNextWindowToNode(Name nodeId)
	{
		ImGuiID dockId = GetDockNodeId(nodeId);
		if (dockId > 0)
		{
			ImGui::SetNextWindowDockID(
			    dockId, bCurrentlyReseting ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
		}
	}

	void DockSpaceLayout::DoReset(ImGuiID dockSpaceID)
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
}    // namespace Rift
