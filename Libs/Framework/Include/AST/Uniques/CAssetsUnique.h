// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "Project.h"

#include <CoreMinimal.h>
#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CAssetsUnique : public Struct
	{
		STRUCT(CAssetsUnique, Struct)

		TAssetPtr<ModuleAsset> rootModule;
		// TArray<TOwnPtr<Project>> subProjects;
	};
}    // namespace Rift
