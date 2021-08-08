// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <CoreObject.h>


namespace Rift
{
	struct CModulesUnique : public Struct
	{
		STRUCT(CModulesUnique, Struct)

		PROP(AST::Id, mainModule)
		AST::Id mainModule = AST::NoId;

		TMap<Name, AST::Id> typesByPath;


		bool HasMainModule() const
		{
			return mainModule != AST::NoId;
		}
	};
}    // namespace Rift
