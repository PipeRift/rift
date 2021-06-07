// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetData.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	enum class ModuleType
	{
		Executable,
		Static,
		Shared,
		Interface
	};

	/**
	 * ModuleAssets represent a group of folders and types to be built together
	 */
	class ModuleAsset : public AssetData
	{
		CLASS(ModuleAsset, AssetData)

	public:
		PROP(Name, name);
		Name name;

		// PROP(ModuleType, type)
		ModuleType type = ModuleType::Executable;

		PROP(String, version)
		String version;
	};
}    // namespace Rift
