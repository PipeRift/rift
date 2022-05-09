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
		explicit OutputId(TAccessRef<struct CExprOutputs, CChild> access, AST::Id pinId);

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

	struct CExprInvalidInputs : public Struct
	{
		STRUCT(CExprInvalidInputs, Struct)

		PROP(pinIds)
		TArray<AST::Id> pinIds;
	};
}    // namespace Rift
