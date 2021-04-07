// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declarations/FunctionDecl.h"

#include <Assets/AssetData.h>
#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	class FunctionLibraryAsset : public AssetData
	{
		CLASS(FunctionLibraryAsset, AssetData)

		TArray<TOwnPtr<FunctionDecl>> functionDecls;
	};
}    // namespace Rift
