// Copyright 2015-2020 Piperift - All rights reserved
#pragma once


#include "Assets/TypeAsset.h"

#include <Containers/Set.h>
#include <Lang/AST.h>
#include <Object/Struct.h>


namespace Rift
{
	/**
	 * This component identifies an AST node as owned by an asset.
	 * Some example are Class, Struct and Function Library declarations pointing to their files
	 */
	struct CTypeAssetRef : public Struct
	{
		STRUCT(CTypeAssetRef, Struct)

		PROP(TAssetPtr<TypeAsset>, asset)
		TAssetPtr<TypeAsset> asset;
	};
}    // namespace Rift
