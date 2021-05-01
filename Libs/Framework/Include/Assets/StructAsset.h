// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"

#include <Lang/AST.h>


namespace Rift
{
	class StructAsset : public TypeAsset
	{
		CLASS(StructAsset, TypeAsset)

		AST::Id declaration;
	};
}    // namespace Rift
