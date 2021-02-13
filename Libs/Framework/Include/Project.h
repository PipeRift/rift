// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Assets/ClassAsset.h"
#include "Assets/ProjectAsset.h"
#include "Assets/StructAsset.h"
#include "Log.h"
#include "Strings/String.h"

#include <Assets/AssetManager.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>
#include <Files/FileSystem.h>


namespace Rift
{
	/**
	 * A project represents a group of classes, structs and other features of the lenguage that can
	 * be edited or compiled
	 */
	class Project : public Object
	{
		CLASS(Project, Object)

		static constexpr StringView projectFile{"Project.rf"};

		Path projectPath;

		TAssetPtr<ProjectAsset> projectAsset;
		TArray<AssetInfo> allTypes;
		TArray<TAssetPtr<ClassAsset>> classes;
		TArray<TAssetPtr<StructAsset>> structs;


	public:
		Project() : Super() {}

		void Init(Path path);

		void ScanAssets();
		void LoadAllAssets();

		Path ToProjectPath(const Path& path) const;

		Path GetPath() const
		{
			return projectPath;
		}

		bool IsValid() const
		{
			return projectAsset.IsValid();
		}

		const TArray<AssetInfo>& GetAllTypeAssets() const
		{
			return allTypes;
		}
	};
}	 // namespace Rift
