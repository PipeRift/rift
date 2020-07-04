// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Assets/ClassAsset.h"
#include "Assets/StructAsset.h"

#include <CoreObject.h>
#include <Files/FileSystem.h>


namespace VCLang
{
	/**
	 * A project represents a group of classes, structs and other features of the lenguage that can be edited or compiled
	 */
	class Project : public Object
	{
		CLASS(Project, Object)

		Path projectPath;

		TArray<Ptr<ClassAsset>> classes;
		TArray<Ptr<StructAsset>> structs;


	public:
		Project() : Super() {}

		void Init(Path path)
		{
			projectPath = FileSystem::ToAbsolute(path, FileSystem::GetCurrent());
		}

		void LoadAllAssets();

		Path ToProjectPath(const Path& path);
	};
}	 // namespace VCLang
