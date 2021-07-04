// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <CoreObject.h>
#include <Memory/OwnPtr.h>


namespace Rift
{
	struct CModulesUnique : public Struct
	{
		STRUCT(CModulesUnique, Struct)

		PROP(AST::Id, mainModule)
		AST::Id mainModule = AST::NoId;


		bool HasMainModule() const
		{
			return mainModule != AST::NoId;
		}
	};
}    // namespace Rift
