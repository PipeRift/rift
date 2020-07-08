// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"

#include "Assets/AssetIterator.h"
#include "Assets/AssetManager.h"


void VCLang::Project::LoadAllAssets()
{
	TArray<AssetInfo> assetInfos;
	for (const auto& asset : AssetIterator<true>(projectPath))
	{
		assetInfos.Add(asset);
	}

	AssetManager::Load(assetInfos);
}

Path VCLang::Project::ToProjectPath(const Path& path)
{
	if (path.is_relative())
	{
		return projectPath / path;
	}
	return FileSystem::IsInside(path, projectPath) ? path : Path{};
}
