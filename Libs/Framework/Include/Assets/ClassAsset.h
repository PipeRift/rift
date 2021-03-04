// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Language/TypeFunction.h"
#include "TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	class ClassAsset : public TypeAsset
	{
		CLASS(ClassAsset, TypeAsset)

		PROP(TArray<TypeFunction>, functions);
		TArray<TypeFunction> functions;
	};
}    // namespace Rift
