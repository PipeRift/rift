// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Filtering.h"
#include "AST/Types.h"
#include "AST/Utils/Hierarchy.h"

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


		OutputId() = default;

		bool IsNone() const
		{
			return ::IsNone(nodeId) || ::IsNone(pinId);
		}
	};

	struct CExprInputs : public Struct
	{
		STRUCT(CExprInputs, Struct)

		PROP(linkedOutputs)
		TArray<OutputId> linkedOutputs;

		PROP(pinIds)
		TArray<AST::Id> pinIds;


		CExprInputs& Add(AST::Id pinId)
		{
			linkedOutputs.AddDefaulted();
			pinIds.Add(pinId);
			return *this;
		}

		void Resize(u32 count)
		{
			linkedOutputs.Resize(count);
			pinIds.Resize(count, AST::NoId);
		}
	};

	struct CExprInvalidInputs : public Struct
	{
		STRUCT(CExprInvalidInputs, Struct)

		PROP(pinIds)
		TArray<AST::Id> pinIds;
	};
}    // namespace Rift
