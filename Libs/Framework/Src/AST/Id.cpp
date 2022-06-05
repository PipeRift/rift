// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"

#include "AST/Serialization.h"

#include <Serialization/Contexts.h>


namespace pipe::ECS
{
	void Read(pipe::ReadContext& ct, pipe::ECS::Id& val)
	{
		auto* astCt = dynamic_cast<rift::AST::ReadContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReadContext")) [[likely]]
		{
			pipe::i32 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetIds()[dataId];
			}
			else
			{
				val = pipe::ECS::NoId;
			}
		}
	}

	void Write(pipe::WriteContext& ct, pipe::ECS::Id val)
	{
		auto* astCt = dynamic_cast<rift::AST::WriteContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriteContext")) [[likely]]
		{
			const pipe::i32* dataId = astCt->GetIdToIndexes().Find(val);
			if (dataId) [[likely]]
			{
				astCt->Serialize(*dataId);
			}
			else
			{
				astCt->Serialize(-1);
			}
		}
	}
}    // namespace pipe::ECS
