// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"

#include "Assets/AssetIterator.h"
#include "Files/Paths.h"

#include <Profiler.h>


namespace Rift
{
	void Project::Init(const Path& path)
	{
		projectPath = Paths::ToAbsolute(path);

		if (!projectPath.empty())
		{
			auto manager = AssetManager::Get();
			projectAsset = manager->Load(AssetInfo(projectPath / projectFile)).Cast<ProjectAsset>();

			if (projectAsset)
			{
				SetName(projectAsset->name);
			}
			else
			{
				SetName(Paths::ToString(projectPath.filename()));
			}

			Log::Info("Project: {}", GetName().ToString());
		}
	}

	void Project::ScanAssets()
	{
		ZoneScopedNC("Find asset files", 0x459bd1);
		allTypes.Empty();
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
		return Paths::IsInside(path, projectPath) ? path : Path{};
	}
}    // namespace Rift
