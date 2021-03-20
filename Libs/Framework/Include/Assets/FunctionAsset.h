// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Language/TypeFunction.h"

#include <Assets/AssetData.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	class FunctionAsset : public AssetData
	{
		CLASS(FunctionAsset, AssetData)

		PROP(TArray<Function>, functions);
		TArray<Function> functions;
	};
}    // namespace Rift
