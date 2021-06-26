// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Module.h"

#include <CoreObject.h>
#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CAssetsUnique : public Struct
	{
		STRUCT(CAssetsUnique, Struct)

		TAssetPtr<ModuleAsset> rootModule;
	};
}    // namespace Rift
