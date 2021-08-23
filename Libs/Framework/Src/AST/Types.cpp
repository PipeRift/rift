// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Types.h"

#include "AST/Serialization.h"

#include <Serialization/Contexts.h>


namespace Rift::Serl
{
	void Read(Serl::ReadContext& ct, AST::Id& val)
	{
		auto* astCt = dynamic_cast<ASTReadContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTReadContext"))
		{
			i64 dataId;
			astCt->Serialize(dataId);

			if (dataId >= 0) [[likely]]
			{
				val = astCt->GetASTIds()[dataId];
			}
			else
			{
				val = AST::NoId;
			}
		}
	}

	void Write(Serl::WriteContext& ct, AST::Id val)
	{
		auto* astCt = dynamic_cast<ASTWriteContext*>(&ct);
		if (EnsureMsg(astCt, "Serializing an AST::Id without an ASTWriteContext"))
		{
			const u32* dataId = astCt->GetASTIdToIndexes().Find(val);
			if (dataId) [[likely]]
			{
				astCt->Serialize(i64(*dataId));
			}
			else
			{
				astCt->Serialize(-1);
			}
		}
	}
}    // namespace Rift::Serl
