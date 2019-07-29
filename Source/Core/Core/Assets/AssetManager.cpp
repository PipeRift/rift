// Copyright 2015-2019 Piperift - All rights reserved

#include "AssetManager.h"

#include "World.h"
#include "Core/Files/FileSystem.h"
#include "Core/Object/ObjectPtr.h"
#include "Core/Engine.h"
#include "Tools/Profiler.h"


Ptr<AssetData> AssetManager::Load(const AssetInfo& info)
{
	Log::Message("Loading asset: %s", info.GetSPath().c_str());

	if (info.IsNull() || !FileSystem::IsAssetPath(info.GetSPath()))
	{
		Log::Error("Invalid asset path '%s'.", info.GetSPath().c_str());
		return {};
	}

	ScopedZone("Asset Load", D19D45);

	Json data;
	String str = info.GetSPath();
	if (FileSystem::LoadJsonFile(info.GetSPath(), data))
	{
		const auto type{ data["asset_type"] };
		if (!type.is_string())
		{
			Log::Error("Asset 'Type' must be an string.");
			return {}; // Asset doesn't have a type
		}

		// Get asset type from json
		Class* assetClass = AssetData::StaticClass()->FindChild(Name{ type });
		if (!assetClass)
		{
			Log::Error("Asset class('%s') not found.", type.get<String>().c_str());
			return {}; // Asset doesn't have a valid class
		}

		// Create asset from json type
		GlobalPtr<AssetData> newAsset = assetClass->CreateInstance(Self()).Cast<AssetData>();

		if (newAsset->OnLoad(info, data))
		{
			const Ptr<AssetData> newAssetPtr = newAsset;

			// Loading succeeded, registry the asset
			loadedAssets[info.GetPath()] = eastl::move(newAsset);
			return eastl::move(newAssetPtr);
		}
	}

	Log::Error("Loading asset failed");
	return {};
}

Ptr<AssetData> AssetManager::LoadOrCreate(const AssetInfo& info, Class* assetType)
{
	if (info.IsNull() || !FileSystem::IsAssetPath(info.GetSPath()))
		return {};

	Ptr<AssetData> loadedAsset = Load(info);
	if(loadedAsset)
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

			loadedAssets[info.GetPath()] = eastl::move(newAsset);

			return newAssetPtr;
		}
	}
	return {};
}


Ptr<AssetManager> AssetManager::Get()
{
	return GEngine->GetAssetManager();
}
