// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"

#include <ECS.h>


namespace Rift
{
	class StructAsset : public TypeAsset
	{
		CLASS(StructAsset, TypeAsset)

		ECS::EntityId declaration;
	};
}    // namespace Rift
