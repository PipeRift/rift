// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct OutputId
	{
		PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		PROP(pinId)
		AST::Id pinId = AST::NoId;
	};

	struct CExprInput : public Struct
	{
		STRUCT(CExprInput, Struct)

		PROP(linkedOutput)
		OutputId linkedOutput;

		PROP(pinId)
		AST::Id pinId = AST::NoId;

		PROP(typeId)
		AST::Id typeId = AST::NoId;
	};

	struct CExprInputs : public Struct
	{
		STRUCT(CExprInputs, Struct)

		PROP(linkedOutputs)
		TArray<OutputId> linkedOutputs;

		PROP(pinIds)
		TArray<AST::Id> pinIds;

		PROP(typeIds)
		TArray<AST::Id> typeids;
	};
}    // namespace Rift
