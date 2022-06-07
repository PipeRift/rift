// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Utils/Hierarchy.h"

#include <Pipe/ECS/Id.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct OutputId : public p::Struct
	{
		STRUCT(OutputId, p::Struct)

		PROP(nodeId)
		AST::Id nodeId = AST::NoId;

		PROP(pinId)
		AST::Id pinId = AST::NoId;


		OutputId() = default;

		bool IsNone() const
		{
			return ecs::IsNone(nodeId) || ecs::IsNone(pinId);
		}
	};

	struct CExprInputs : public p::Struct
	{
		STRUCT(CExprInputs, p::Struct)

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

		CExprInputs& Insert(i32 index, AST::Id pinId)
		{
			pinIds.Insert(index, pinId);
			linkedOutputs.Insert(index, {});
			return *this;
		}

		CExprInputs& Swap(i32 firstIndex, i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			linkedOutputs.Swap(firstIndex, secondIndex);
			return *this;
		}

		void Resize(i32 count)
		{
			linkedOutputs.Resize(count);
			pinIds.Resize(count, AST::NoId);
		}
	};
}    // namespace rift
