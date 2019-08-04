// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Core/Assets/AssetData.h"


class ModuleAsset : public AssetData
{
	CLASS(ModuleAsset, BaseAsset)

	P(String, path);
	
	P(TArray<AssetPtr<BaseAsset>>, cachedAssets, Transient);
};
