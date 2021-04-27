// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "TypeAsset.h"

#include <ECS.h>


namespace Rift
{
	class ClassAsset : public TypeAsset
	{
		CLASS(ClassAsset, TypeAsset)

		ECS::EntityId astDeclaration;
	};
}    // namespace Rift
