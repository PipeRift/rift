// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "AssetData.h"
#include "Containers/Map.h"
#include "CoreObject.h"
#include "Object/ObjectPtr.h"


class AssetManager : public Object
{
	CLASS(AssetManager, Object)

private:
	TArray<AssetInfo> assetInfos;

	TMap<Name, GlobalPtr<AssetData>> loadedAssets{};


public:
	Ptr<AssetData> Load(const AssetInfo& info);

	Ptr<AssetData> LoadOrCreate(const AssetInfo& info, Class* assetType);

	inline Ptr<AssetData> GetLoadedAsset(const AssetInfo& id)
	{
		return loadedAssets[id.GetPath()];
	}

	static Ptr<AssetManager> Get();
};
