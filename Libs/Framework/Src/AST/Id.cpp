// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Id.h"

#include "AST/Serialization.h"

#include <Serialization/Serialization.h>


namespace p::ecs
{
	void Read(Reader& ct, Id& val)
	{
		auto* astCt = dynamic_cast<rift::AST::Reader*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReader")) [[likely]]
		{
			i32 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetIds()[dataId];
			}
			else
			{
				val = NoId;
			}
		}
	}

	void Write(Writer& ct, Id val)
	{
		auto* astCt = dynamic_cast<rift::AST::Writer*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriter")) [[likely]]
		{
			const i32* dataId = astCt->GetIdToIndexes().Find(val);
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
