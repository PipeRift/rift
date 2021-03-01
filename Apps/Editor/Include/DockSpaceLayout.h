// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "UI/UI.h"

#include <Containers/Map.h>
#include <Events/Function.h>
#include <Strings/Name.h>


struct DockSpaceLayout
{
	static const Rift::Name rootNodeId;


	struct Builder
	{
	private:
		DockSpaceLayout& layout;

	public:
		Builder(DockSpaceLayout& layout) : layout(layout) {}


		bool Split(Rift::Name originNodeId, ImGuiDir direction, float splitRatio,
		    Rift::Name newNodeID, Rift::Name oppositeNewNodeID);

		ImGuiDockNodeFlags& GetNodeLocalFlags(Rift::Name nodeId);
		ImGuiDockNodeFlags& GetNodeSharedFlags(Rift::Name nodeId);

		static Rift::Name GetRootNode()
		{
			return rootNodeId;
		}
	};

	friend Builder;


private:
	bool bWantsToReset      = false;
	bool bCurrentlyReseting = false;
	Rift::TMap<Rift::Name, ImGuiID> nameToDockNodeId;
	Rift::TFunction<void(Builder&)> onBuild{};


public:
	DockSpaceLayout() = default;

	DockSpaceLayout& OnBuild(Rift::TFunction<void(Builder&)> callback)
	{
		onBuild = callback;
		return *this;
	}

	void Tick(ImGuiID dockSpaceID);

	void Reset()
	{
		bWantsToReset = true;
	}

	void BindWindowToNode(Rift::StringView windowId, Rift::Name nodeId);
	void BindNextWindowToNode(Rift::Name nodeId);

	bool WantsToReset() const
	{
		return bWantsToReset;
	}

private:
	void DoReset(ImGuiID dockSpaceID);

	ImGuiID GetDockNodeId(Rift::Name nameId) const
	{
		const ImGuiID* const idPtr = nameToDockNodeId.Find(nameId);
		return idPtr ? *idPtr : 0;
	}
};
