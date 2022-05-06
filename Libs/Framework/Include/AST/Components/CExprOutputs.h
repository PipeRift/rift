// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct CExprOutputs : public Struct
	{
		STRUCT(CExprOutputs, Struct)

		PROP(pinIds)
		TArray<AST::Id> pinIds;

		PROP(typeIds)
		TArray<AST::Id> typeIds;

		CExprOutputs() {}
		CExprOutputs(AST::Id pinId, AST::Id typeId)
		{
			AddPin(pinId, typeId);
		}

		CExprOutputs& AddPin(AST::Id pinId, AST::Id typeId)
		{
			pinIds.Add(pinId);
			typeIds.Add(typeId);
			return *this;
		}
	};

	struct CExprInvalidOutputs : public Struct
	{
		STRUCT(CExprInvalidOutputs, Struct)

		PROP(pinIds)
		TArray<AST::Id> pinIds;
	};
}    // namespace Rift
