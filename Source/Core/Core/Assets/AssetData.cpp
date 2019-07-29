// © 2019 Miguel Fernández Arce - All rights reserved

#include "AssetData.h"
#include "Core/Files/FileSystem.h"



bool AssetData::OnCreate(const AssetInfo& inInfo)
{
	info = inInfo;
	return PostLoad();
}

bool AssetData::OnLoad(const AssetInfo& inInfo, Json& data)
{
	info = inInfo;

	// Deserialize asset
	JsonArchive ar(data);
	Serialize(ar);

	return PostLoad();
}

bool AssetData::SaveToPath(const Name& path)
{
	const String& sPath = path.ToString();
	if (sPath.empty() || !FileSystem::IsAssetPath(sPath))
		return false;

	JsonArchive ar{};
	Name className = GetClass()->GetName();
	ar("asset_type", className);
	Serialize(ar);

	return FileSystem::SaveJsonFile(sPath, ar.GetData(), ar.GetIndent());
}

bool AssetData::Save()
{
	return SaveToPath(info.GetPath());
}
