// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"

#include "AST/Serialization.h"

#include <Serialization/Contexts.h>


namespace Pipe::ECS
{
	void Read(Pipe::ReadContext& ct, Pipe::ECS::Id& val)
	{
		auto* astCt = dynamic_cast<Rift::AST::ReadContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReadContext")) [[likely]]
		{
			Pipe::i32 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetIds()[dataId];
			}
			else
			{
				val = Pipe::ECS::NoId;
			}
		}
	}

	void Write(Pipe::WriteContext& ct, Pipe::ECS::Id val)
	{
		auto* astCt = dynamic_cast<Rift::AST::WriteContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriteContext")) [[likely]]
		{
			const Pipe::i32* dataId = astCt->GetIdToIndexes().Find(val);
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
}    // namespace Pipe::ECS
