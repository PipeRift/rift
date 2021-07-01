// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Assets/ModuleAsset.h"

#include <CoreObject.h>
#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CModulesUnique : public Struct
	{
		STRUCT(CModulesUnique, Struct)

		TAssetPtr<ModuleAsset> main;
		TArray<TAssetPtr<ModuleAsset>> dependencies;


		bool HasMainModule() const
		{
			return main.IsValid();
		}
	};
}    // namespace Rift
