
#pragma once

#include <CoreObject.h>
#include <Files/FileSystem.h>

#include "Assets/TypeAsset.h"


namespace VCLang
{
	/**
	 * A project represents a group of classes, structs and other features of the lenguage that can be edited or compiled
	 */
	class Project : public Object
	{
		CLASS(Project, Object)

		Path path;

		// Classes and Structures
		TArray<GlobalPtr<TypeAsset>> types;


public:

		Project() : Super() {}

		void Init(Path inPath)
		{
			path = inPath;
		}
	};
}
