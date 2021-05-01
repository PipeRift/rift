// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "TypeAsset.h"

#include <Lang/AST.h>


namespace Rift
{
	class ClassAsset : public TypeAsset
	{
		CLASS(ClassAsset, TypeAsset)

		AST::Id declaration;
	};
}    // namespace Rift
