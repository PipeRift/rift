// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"
#include "Editors/BaseEditor.h"
#include "NodeGraph/NodeGraphPanel.h"
#include "UI/UI.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


namespace Rift
{
	class AssetEditor : public BaseEditor
	{
		CLASS(AssetEditor, BaseEditor)
	public:
		static const Name rightNode;
		static const Name centralNode;

	private:
		TAssetPtr<TypeAsset> asset;

		ImGuiID dockspaceID = 0;
		DockSpaceLayout layout;

		NodeGraphPanel nodeGraph{};


	public:
		AssetEditor();

		void SetAsset(TAssetPtr<TypeAsset> inAsset)
		{
			if (asset)
			{
				return;    // Can only assign an asset once
			}
			asset = inAsset;
		}

		void Draw();

		TAssetPtr<TypeAsset> GetAsset() const
		{
			return asset;
		}

		DockSpaceLayout& GetLayout()
		{
			return layout;
		}
		const DockSpaceLayout& GetLayout() const
		{
			return layout;
		}

	private:
		void CreateDockspace();
	};
}    // namespace Rift
