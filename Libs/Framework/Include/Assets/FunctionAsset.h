// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetData.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>
#include <ECS.h>


namespace Rift
{
	class FunctionLibraryAsset : public AssetData
	{
		CLASS(FunctionLibraryAsset, AssetData)

		TArray<ECS::EntityId> functionDeclarations;
	};
}    // namespace Rift
