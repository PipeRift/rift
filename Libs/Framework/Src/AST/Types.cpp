// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Types.h"

#include "AST/Serialization.h"

#include <Serialization/Contexts.h>


namespace Rift::Serl
{
	void Read(Serl::ReadContext& ct, AST::Id& val)
	{
		auto* astCt = dynamic_cast<AST::ReadContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReadContext")) [[likely]]
		{
			i32 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetIds()[dataId];
			}
			else
			{
				val = AST::NoId;
			}
		}
	}

	void Write(Serl::WriteContext& ct, AST::Id val)
	{
		auto* astCt = dynamic_cast<AST::WriteContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriteContext")) [[likely]]
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
}    // namespace Rift::Serl
