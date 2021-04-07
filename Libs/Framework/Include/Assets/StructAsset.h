// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"
#include "Lang/Declarations/StructDecl.h"


namespace Rift
{
	class StructAsset : public TypeAsset
	{
		CLASS(StructAsset, TypeAsset)

		TOwnPtr<StructDecl> declaration;
	};
}    // namespace Rift
