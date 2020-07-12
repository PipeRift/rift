// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AssetData.h"
#include "AssetInfo.h"
#include "Containers/Map.h"
#include "CoreObject.h"
#include "Files/FileSystem.h"
#include "Object/ObjectPtr.h"


namespace VCLang
{
	class AssetManager : public Object
	{
		CLASS(AssetManager, Object)

	public:
		static constexpr StringView assetFormat{".vc"};

	private:
		TArray<AssetInfo> assetInfos;

		TMap<AssetInfo, GlobalPtr<AssetData>> loadedAssets{};


	public:
		Ptr<AssetData> Load(AssetInfo info);
		TArray<Ptr<AssetData>> Load(TArray<AssetInfo> infos);

		Ptr<AssetData> LoadOrCreate(AssetInfo info, Refl::Class* assetType);

		Ptr<AssetData> GetLoadedAsset(AssetInfo id) const
		{
			if (const auto* asset = loadedAssets.Find(id))
			{
				return asset->AsPtr();
			}
			return {};
		}

		bool IsLoaded(AssetInfo id) const
		{
			return loadedAssets.Contains(id);
		}

		static Ptr<AssetManager> Get();
	};
}	 // namespace VCLang
