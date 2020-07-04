// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "AssetData.h"
#include "Containers/Map.h"
#include "Object/ObjectPtr.h"


class AssetManager : public Object
{
	CLASS(AssetManager, Object)

public:
	static const StringView assetFormat;

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
