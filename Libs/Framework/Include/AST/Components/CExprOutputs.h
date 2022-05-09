// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Filtering.h"
#include "AST/Types.h"
#include "AST/Utils/Hierarchy.h"

#include <Types/Struct.h>


namespace Rift
{
	struct InputId : public Struct
	{
		STRUCT(InputId, Struct)

		PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		PROP(pinId)
		AST::Id pinId = AST::NoId;


		InputId() = default;
		explicit InputId(TAccessRef<struct CExprInputs, CChild> access, AST::Id pinId);

		bool IsNone() const
		{
			return ::IsNone(nodeId) || ::IsNone(pinId);
		}
	};

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
