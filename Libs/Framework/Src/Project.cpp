// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"

#include "Assets/AssetIterator.h"

#include <Profiler.h>


namespace Rift
{
	void Project::Init(Path path)
	{
		projectPath = FileSystem::ToAbsolute(path);

		if (!projectPath.empty())
		{
			auto manager = AssetManager::Get();
			projectAsset = manager->Load(AssetInfo(projectPath / projectFile)).Cast<ProjectAsset>();

			if (projectAsset)
			{
				SetName(projectAsset->GetName());
			}
			else
			{
				SetName(FileSystem::ToString(projectPath.filename()));
			}

			Log::Info("Project: {}", GetName().ToString());
		}
	}

	void Project::ScanAssets()
	{
		ZoneScopedNC("Find asset files", 0x459bd1);
		for (const auto& asset : AssetIterator<true>(projectPath))
		{
			allTypes.Add(asset);
		}
	}

	void Project::LoadAllAssets()
	{
		ZoneScopedC(0x459bd1);
		ScanAssets();
		AssetManager::Get()->Load(allTypes);
	}

	Path Project::ToProjectPath(const Path& path) const
	{
		if (path.is_relative())
		{
			return projectPath / path;
		}
		return FileSystem::IsInside(path, projectPath) ? path : Path{};
	}
}	 // namespace Rift
