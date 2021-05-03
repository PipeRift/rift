// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetData.h>
#include <CoreObject.h>
#include <Lang/AST.h>
#include <Reflection/ClassTraits.h>


namespace Rift
{
	class TypeAsset : public AssetData
	{
		CLASS(TypeAsset, AssetData)

	public:
		AST::Id declaration;


		void Serialize(Archive& ar, StringView name);
	};

	DEFINE_CLASS_TRAITS(TypeAsset, HasCustomSerialize = true);
}    // namespace Rift

