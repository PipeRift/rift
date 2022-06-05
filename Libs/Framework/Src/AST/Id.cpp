// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"

#include "AST/Serialization.h"

#include <Serialization/Contexts.h>


namespace p::ecs
{
	void Read(p::ReadContext& ct, p::ecs::Id& val)
	{
		auto* astCt = dynamic_cast<rift::AST::ReadContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReadContext")) [[likely]]
		{
			p::i32 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetIds()[dataId];
			}
			else
			{
				val = p::ecs::NoId;
			}
		}
	}

	void Write(p::WriteContext& ct, p::ecs::Id val)
	{
		auto* astCt = dynamic_cast<rift::AST::WriteContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriteContext")) [[likely]]
		{
			const p::i32* dataId = astCt->GetIdToIndexes().Find(val);
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
}    // namespace p::ecs
