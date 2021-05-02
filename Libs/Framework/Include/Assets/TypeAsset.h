// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetData.h>
#include <CoreObject.h>
#include <Lang/AST.h>


namespace Rift
{
	class TypeAsset : public AssetData
	{
		CLASS(TypeAsset, AssetData)

		AST::Id declaration;
	};
}    // namespace Rift
