// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declarations/ClassDecl.h"
#include "TypeAsset.h"

#include <Assets/AssetPtr.h>
#include <CoreObject.h>


namespace Rift
{
	class ClassAsset : public TypeAsset
	{
		CLASS(ClassAsset, TypeAsset)

		TOwnPtr<ClassDecl> declaration;
	};
}    // namespace Rift
