// Copyright 2015-2020 Piperift - All rights reserved

#include "Assets/AssetIterator.h"
#include "Files/Paths.h"
#include "Module.h"

#include <Profiler.h>


namespace Rift
{
	void Module::Init(const Path& inPath)
	{
		path = Paths::ToAbsolute(inPath);

		if (!path.empty())
		{
			auto manager = AssetManager::Get();
			asset        = manager->Load(AssetInfo(path / file)).Cast<ModuleAsset>();

			if (asset)
			{
				SetName(asset->name);
			}
			else
			{
				SetName(Paths::ToString(path.filename()));
			}

			Log::Info("Project: {}", GetName().ToString());
		}
	}

	void Module::ScanAssets()
	{
		ZoneScopedNC("Find asset files", 0x459bd1);
		allTypes.Empty();
		for (const auto& asset : AssetIterator<true>(path))
		{
			allTypes.Add(asset);
		}
	}

	void Module::LoadAllAssets()
	{
		ZoneScopedC(0x459bd1);
		ScanAssets();
		AssetManager::Get()->Load(allTypes);
	}

	Path Module::ToModulePath(const Path& inPath) const
	{
		if (inPath.is_relative())
		{
			return path / inPath;
		}
		return Paths::IsInside(path, inPath) ? path : Path{};
	}
}    // namespace Rift
