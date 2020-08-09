// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"

#include "Assets/AssetIterator.h"

#include <Profiler.h>


namespace VCLang
{
	void Project::Init(Path path)
	{
		projectPath = FileSystem::ToAbsolute(path);

		if (!projectPath.empty())
		{
			auto assets = AssetManager::Get();
			asset = assets->Load(AssetInfo(projectPath / projectFile)).Cast<ProjectAsset>();

			if (asset)
			{
				SetName(asset->GetName());
			}
			else
			{
				SetName(FileSystem::ToString(projectPath.filename()));
			}

			Log::Info("Project: {}", GetName().ToString());
		}
	}

	void Project::LoadAllAssets()
	{
		ZoneScopedC(0x459bd1);
		TArray<AssetInfo> assetInfos;
		{
			ZoneScopedNC("Find asset files", 0x459bd1);
			for (const auto& asset : AssetIterator<true>(projectPath))
			{
				assetInfos.Add(asset);
			}
		}
		AssetManager::Get()->Load(assetInfos);
	}

	Path Project::ToProjectPath(const Path& path) const
	{
		if (path.is_relative())
		{
			return projectPath / path;
		}
		return FileSystem::IsInside(path, projectPath) ? path : Path{};
	}
}	 // namespace VCLang
