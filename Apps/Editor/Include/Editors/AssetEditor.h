// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "DockSpaceLayout.h"
#include "Editors/BaseEditor.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>
#include <imgui.h>


using namespace Rift;

class AssetEditor : public BaseEditor
{
	CLASS(AssetEditor, BaseEditor)

	static const Name rightNode;
	static const Name centralNode;

	TAssetPtr<TypeAsset> asset;

	ImGuiID dockspaceID = 0;
	DockSpaceLayout layout;


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
