// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Function.h>
#include <Core/Map.h>
#include <Core/Name.h>
#include <UI/UI.h>


namespace rift
{
	using namespace pipe;

	struct DockSpaceLayout
	{
		static const Name rootNodeId;


		struct Builder
		{
		private:
			DockSpaceLayout& layout;

		public:
			Builder(DockSpaceLayout& layout) : layout(layout) {}


			bool Split(Name originNodeId, ImGuiDir direction, float splitRatio, Name newNodeID,
			    Name oppositeNewNodeID);

			ImGuiDockNodeFlags& GetNodeLocalFlags(Name nodeId);
			ImGuiDockNodeFlags& GetNodeSharedFlags(Name nodeId);

			static Name GetRootNode()
			{
				return rootNodeId;
			}
		};

		friend Builder;


	private:
		bool bWantsToReset      = false;
		bool bCurrentlyReseting = false;
		TMap<Name, ImGuiID> nameToDockNodeId;
		TFunction<void(Builder&)> onBuild{};


	public:
		DockSpaceLayout() = default;

		DockSpaceLayout& OnBuild(TFunction<void(Builder&)> callback)
		{
			onBuild = callback;
			return *this;
		}

		void Tick(ImGuiID dockSpaceID);

		void Reset()
		{
			bWantsToReset = true;
		}

		void BindWindowToNode(StringView windowId, Name nodeId);
		void BindNextWindowToNode(Name nodeId, ImGuiCond cond = ImGuiCond_FirstUseEver);

	private:
		void Rebuild(ImGuiID dockSpaceID);

		ImGuiID GetDockNodeId(Name nameId) const
		{
			const ImGuiID* const idPtr = nameToDockNodeId.Find(nameId);
			return idPtr ? *idPtr : 0;
		}
	};
}    // namespace rift
