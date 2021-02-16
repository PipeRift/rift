// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"

#include <Log.h>
#include <imgui_internal.h>


const Rift::Name DockSpaceLayout::rootNodeId{"root"};


bool DockSpaceLayout::Builder::Split(Rift::Name originNodeId, ImGuiDir direction, float splitRatio,
    Rift::Name newNodeID, Rift::Name oppositeNewNodeID)
{
	const ImGuiID originDockNodeId = layout.GetDockNodeId(originNodeId);
	if (originDockNodeId == 0)
	{
		Rift::Log::Error("Error splitting node {}. It doesn't exist.", originNodeId.ToString());
		return false;
	}

	ImGuiID newID = 0, oppositeNewID = 0;
	ImGui::DockBuilderSplitNode(originDockNodeId, direction, splitRatio, &newID, &oppositeNewID);

	layout.nameToDockNodeId.Insert(newNodeID, newID);
	layout.nameToDockNodeId.Insert(oppositeNewNodeID, oppositeNewID);
	return true;
}

ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeLocalFlags(Rift::Name nodeId)
{
	const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
	assert(dockNodeId > 0);
	return ImGui::DockBuilderGetNode(dockNodeId)->LocalFlags;
}

ImGuiDockNodeFlags& DockSpaceLayout::Builder::GetNodeSharedFlags(Rift::Name nodeId)
{
	const ImGuiID dockNodeId = layout.GetDockNodeId(nodeId);
	assert(dockNodeId > 0);
	return ImGui::DockBuilderGetNode(dockNodeId)->SharedFlags;
}


void DockSpaceLayout::Tick(ImGuiID dockSpaceID)
{
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

void DockSpaceLayout::BindWindowToNode(Rift::StringView windowId, Rift::Name nodeId)
{
	ImGuiID dockId = GetDockNodeId(nodeId);
	if (dockId > 0)
	{
		ImGui::DockBuilderDockWindow(windowId.data(), dockId);
	}
}
void DockSpaceLayout::BindNextWindowToNode(Rift::Name nodeId)
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
	// Keep root id always updated
	nameToDockNodeId.Insert(rootNodeId, dockSpaceID);

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