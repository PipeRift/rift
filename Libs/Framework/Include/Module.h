// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "Assets/ModuleAsset.h"
#include "Assets/TypeAsset.h"
#include "Log.h"
#include "Strings/String.h"

#include <Assets/AssetManager.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>
#include <Files/Files.h>


namespace Rift
{
	enum class EProjectTargetType : u8
	{
		Executable,
		SharedLibrary,
		StaticLibrary
	};
	ENUM(EProjectTargetType)

	enum class EOther : u8
	{
		Executable,
		SharedLibrary,
		StaticLibrary
	};

	/**
	 * A project represents a group of classes, structs and other features of the lenguage that can
	 * be edited or compiled
	 */
	/*class Module : public Object
	{
		CLASS(Module, Object)

		static constexpr StringView file{"Project.rift"};

		PROP(Name, name);
		Name name;

		Path path;

		TAssetPtr<ModuleAsset> asset;
		TArray<AssetInfo> allTypes;


	public:
		Module() : Super() {}

		void Init(const Path& path);

		void ScanAssets();
		void LoadAllAssets();

		Path ToModulePath(const Path& path) const;

		Path GetPath() const
		{
			return path;
		}

		bool IsValid() const
		{
			return asset.IsValid();
		}

		const TArray<AssetInfo>& GetAllTypeAssets() const
		{
			return allTypes;
		}

		void SetName(Name newName)
		{
			name = Move(newName);
		}
		Name GetName() const
		{
			return name;
		}
	};*/
}    // namespace Rift
