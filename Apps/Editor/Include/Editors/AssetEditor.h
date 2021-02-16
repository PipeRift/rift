// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Editors/BaseEditor.h"

#include <Assets/AssetPtr.h>
#include <Assets/TypeAsset.h>


using namespace Rift;

class AssetEditor : public BaseEditor
{
	CLASS(AssetEditor, BaseEditor)

	TAssetPtr<TypeAsset> asset;

public:
	AssetEditor() : Super() {}

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
};
