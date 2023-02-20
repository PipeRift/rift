// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Function.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/Tag.h>
#include <UI/UI.h>


namespace rift::Editor
{
	using namespace p;

	struct DockSpaceLayout
	{
		static const Tag rootNodeId;


		struct Builder
		{
		private:
			DockSpaceLayout& layout;

		public:
			Builder(DockSpaceLayout& layout) : layout(layout) {}


			bool Split(Tag originNodeId, ImGuiDir direction, float splitRatio, Tag newNodeID,
			    Tag oppositeNewNodeID);

			ImGuiDockNodeFlags& GetNodeLocalFlags(Tag nodeId);
			ImGuiDockNodeFlags& GetNodeSharedFlags(Tag nodeId);

			static Tag GetRootNode()
			{
				return rootNodeId;
			}
		};

		friend Builder;


	private:
		bool bWantsToReset      = false;
		bool bCurrentlyReseting = false;
		TMap<Tag, ImGuiID> nameToDockNodeId;
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

		void BindWindowToNode(StringView windowId, Tag nodeId);
		void BindNextWindowToNode(Tag nodeId, ImGuiCond cond = ImGuiCond_FirstUseEver);

	private:
		void Rebuild(ImGuiID dockSpaceID);

		ImGuiID GetDockNodeId(Tag nameId) const
		{
			const ImGuiID* const idPtr = nameToDockNodeId.Find(nameId);
			return idPtr ? *idPtr : 0;
		}
	};
}    // namespace rift::Editor
