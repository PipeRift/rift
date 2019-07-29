// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "Core/Object/ObjectPtr.h"
#include "Core/Containers/Map.h"

#include "AssetData.h"


class AssetManager : public Object {
	CLASS(AssetManager, Object)

private:

	TArray<AssetInfo> assetInfos;

	TMap<Name, GlobalPtr<AssetData>> loadedAssets { Name::None() };


public:

	Ptr<AssetData> Load(const AssetInfo& info);
	Ptr<AssetData> LoadOrCreate(const AssetInfo& info, Class* assetType);


	inline Ptr<AssetData> GetLoadedAsset(const AssetInfo& id) {
		return loadedAssets[id.GetPath()];
	}

	static Ptr<AssetManager> Get();
};
