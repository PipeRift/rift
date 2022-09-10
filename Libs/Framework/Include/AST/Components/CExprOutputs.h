// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/ECS/Access.h>
#include <Pipe/ECS/Id.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct ExprInput : public p::Struct
	{
		STRUCT(ExprInput, p::Struct)

		PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		PROP(pinId)
		AST::Id pinId = AST::NoId;


		ExprInput() = default;

		bool IsNone() const
		{
			return p::ecs::IsNone(nodeId) || p::ecs::IsNone(pinId);
		}
	};

	struct CExprOutputs : public p::Struct
	{
		STRUCT(CExprOutputs, p::Struct)

		PROP(pinIds)
		p::TArray<AST::Id> pinIds;


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

		CExprOutputs& Insert(p::i32 index, AST::Id pinId)
		{
			pinIds.Insert(index, pinId);
			return *this;
		}

		CExprOutputs& Swap(p::i32 firstIndex, p::i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			return *this;
		}
	};
}    // namespace rift::AST
