// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"

#include "Assets/AssetIterator.h"
#include "Assets/AssetManager.h"

#include <Profiler.h>


void VCLang::Project::LoadAllAssets()
{
	ScopedStackZone(459bd1);
	TArray<AssetInfo> assetInfos;
	{
		ScopedZone("Find asset files", 459bd1);
		for (const auto& asset : AssetIterator<true>(projectPath))
		{
			assetInfos.Add(asset);
		}
	}
	AssetManager::Get()->Load(assetInfos);
}

Path VCLang::Project::ToProjectPath(const Path& path)
{
	if (path.is_relative())
	{
		return projectPath / path;
	}
	return FileSystem::IsInside(path, projectPath) ? path : Path{};
}
