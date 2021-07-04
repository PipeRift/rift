// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <Object/Struct.h>


namespace Rift
{
	struct CModule : public Struct
	{
		STRUCT(CModule, Struct)

		PROP(bool, isMain, Transient)
		bool isMain = false;

		PROP(TAssetPtr<ModuleAsset>, asset, Transient)
		TAssetPtr<ModuleAsset> asset;


		CModule(bool isMain, TAssetPtr<ModuleAsset> asset) : isMain{isMain}, asset{asset} {}
	};
}    // namespace Rift
