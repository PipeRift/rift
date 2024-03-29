// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Pipe/PipeECS.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct ExprOutput : public p::Struct
	{
		STRUCT(ExprOutput, p::Struct)

		PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		PROP(pinId)
		AST::Id pinId = AST::NoId;


		ExprOutput() = default;

		bool IsNone() const
		{
			return p::IsNone(nodeId) || p::IsNone(pinId);
		}
	};

	struct CExprInputs : public p::Struct
	{
		STRUCT(CExprInputs, p::Struct)

		PROP(linkedOutputs)
		p::TArray<ExprOutput> linkedOutputs;

		PROP(pinIds)
		p::TArray<AST::Id> pinIds;


		CExprInputs& Add(AST::Id pinId)
		{
			linkedOutputs.Add();
			pinIds.Add(pinId);
			return *this;
		}

		CExprInputs& Insert(p::i32 index, AST::Id pinId)
		{
			pinIds.Insert(index, pinId);
			linkedOutputs.Insert(index);
			return *this;
		}

		CExprInputs& Swap(p::i32 firstIndex, p::i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			linkedOutputs.Swap(firstIndex, secondIndex);
			return *this;
		}

		void Resize(p::i32 count)
		{
			linkedOutputs.Resize(count);
			pinIds.Resize(count, AST::NoId);
		}
	};
}    // namespace rift::AST
