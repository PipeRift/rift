// Copyright 2015-2020 Piperift - All rights reserved

#include "Assets/AssetManager.h"

#include "Context.h"
#include "Files/FileSystem.h"
#include "Object/ObjectPtr.h"
#include "Profiler.h"


const StringView AssetManager::assetFormat = "vc";


Ptr<AssetData> AssetManager::Load(const AssetInfo& info)
{
	Log::Message("Loading asset: %s", info.GetStrPath().c_str());

	if (info.IsNull() || !FileSystem::IsFile(info.GetStrPath()))
	{
		Log::Error("Invalid asset path '%s'.", info.GetStrPath().c_str());
		return {};
	}

	ScopedZone("Asset Load", D19D45);

	Json data;
	if (FileSystem::LoadJsonFile(info.GetStrPath(), data))
	{
		const auto type{data["asset_type"]};
		if (!type.is_string())
		{
			Log::Error("Asset 'Type' must be an string.");
			return {};	  // Asset doesn't have a type
		}

		// Get asset type from json
		Class* assetClass = AssetData::StaticClass()->FindChild(Name{type});
		if (!assetClass)
		{
			Log::Error("Asset class('%s') not found.", type.get<String>().c_str());
			return {};	  // Asset doesn't have a valid class
		}

		// Create asset from json type
		GlobalPtr<AssetData> newAsset = assetClass->CreateInstance(Self()).Cast<AssetData>();

		if (newAsset->OnLoad(info, data))
		{
			const Ptr<AssetData> newAssetPtr = newAsset;

			// Loading succeeded, registry the asset
			loadedAssets[info.GetPath()] = MoveTemp(newAsset);
			return MoveTemp(newAssetPtr);
		}
	}

	Log::Error("Loading asset failed");
	return {};
}

Ptr<AssetData> AssetManager::LoadOrCreate(const AssetInfo& info, Class* assetType)
{
	if (info.IsNull() || !FileSystem::IsFolder(info.GetStrPath()))
	{
		return {};
	}

	Ptr<AssetData> loadedAsset = Load(info);
	if (loadedAsset)
	{
		return loadedAsset;
	}

	if (!assetType)
		return {};

	if (assetType->IsChildOf<AssetData>())
	{
		GlobalPtr<AssetData> newAsset = assetType->CreateInstance(Self()).Cast<AssetData>();
		if (newAsset->OnCreate(info))
		{
			const Ptr<AssetData> newAssetPtr = newAsset;

			loadedAssets[info.GetPath()] = MoveTemp(newAsset);

			return newAssetPtr;
		}
	}
	return {};
}


Ptr<AssetManager> AssetManager::Get()
{
	return Context::Get()->GetAssetManager();
}
