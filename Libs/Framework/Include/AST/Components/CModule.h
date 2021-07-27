// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <Containers/Map.h>
#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CModule : public Struct
	{
		STRUCT(CModule, Struct)

		PROP(bool, isMain, Transient)
		bool isMain = false;

		using TypeMap = TMap<Guid, AST::Id>;
		// TODO: Reflect Maps
		// PROP(TypeMap, types, Transient)
		TypeMap types;

		// TODO: Remove when deprecating assets
		PROP(TAssetPtr<ModuleAsset>, asset, Transient)
		TAssetPtr<ModuleAsset> asset;

		PROP(Path, path, Transient)
		Path path;


		CModule(bool isMain, TAssetPtr<ModuleAsset> asset) : isMain{isMain}, asset{asset}
		{
			auto parentPath = Paths::GetParent(asset.GetStrPath());
			path            = Paths::FromString(parentPath);
		}
	};
}    // namespace Rift
