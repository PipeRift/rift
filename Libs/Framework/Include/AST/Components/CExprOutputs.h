// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Hierarchy.h"

#include <ECS/Access.h>
#include <ECS/Id.h>
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

		bool IsNone() const
		{
			return ECS::IsNone(nodeId) || ECS::IsNone(pinId);
		}
	};

	struct CExprOutputs : public Struct
	{
		STRUCT(CExprOutputs, Struct)

		PROP(pinIds)
		TArray<AST::Id> pinIds;


		CExprOutputs() {}
		CExprOutputs(AST::Id pinId)
		{
			Add(pinId);
		}

		CExprOutputs& Add(AST::Id pinId)
		{
			pinIds.Add(pinId);
			return *this;
		}

		CExprOutputs& Insert(i32 index, AST::Id pinId)
		{
			pinIds.Insert(index, pinId);
			return *this;
		}

		CExprOutputs& Swap(i32 firstIndex, i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			return *this;
		}
	};
}    // namespace Rift
