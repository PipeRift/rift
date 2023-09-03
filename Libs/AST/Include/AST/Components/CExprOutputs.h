// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/PipeECS.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct ExprInput : public p::Struct
	{
		P_STRUCT(ExprInput, p::Struct)

		P_PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		P_PROP(pinId)
		AST::Id pinId = AST::NoId;


		ExprInput() = default;

		bool IsNone() const
		{
			return p::IsNone(nodeId) || p::IsNone(pinId);
		}
	};

	struct CExprOutputs : public p::Struct
	{
		P_STRUCT(CExprOutputs, p::Struct)

		P_PROP(pinIds)
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
