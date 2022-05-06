// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Types/Struct.h>


namespace Rift
{
	struct OutputId : public Struct
	{
		STRUCT(OutputId, Struct)

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


		CExprInput(AST::Id pinId, AST::Id typeId) : pinId{pinId}, typeId{typeId} {}
	};

	struct CExprInputs : public Struct
	{
		STRUCT(CExprInputs, Struct)

		PROP(linkedOutputs)
		TArray<OutputId> linkedOutputs;

		PROP(pinIds)
		TArray<AST::Id> pinIds;

		PROP(typeIds)
		TArray<AST::Id> typeIds;


		CExprInputs& AddPin(AST::Id pinId, AST::Id typeId)
		{
			linkedOutputs.AddDefaulted();
			pinIds.Add(pinId);
			typeIds.Add(typeId);
			return *this;
		}

		void Resize(u32 count)
		{
			linkedOutputs.Resize(count);
			pinIds.Resize(count, AST::NoId);
			typeIds.Resize(count, AST::NoId);
		}
	};
}    // namespace Rift
