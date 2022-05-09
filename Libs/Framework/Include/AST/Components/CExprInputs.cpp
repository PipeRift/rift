// Copyright 2015-2022 Piperift - All rights reserved

#include "CExprInputs.h"

#include "CExprOutputs.h"


namespace Rift
{
	OutputId::OutputId(TAccessRef<CExprOutputs, CChild> access, AST::Id pinId) : pinId(pinId)
	{
		nodeId = pinId;
		// If node is not the pin itself, it must be the parent
		if (!IsNone(nodeId) && !access.Has<CExprOutputs>(nodeId))
		{
			nodeId = AST::Hierarchy::GetParent(access, pinId);
		}
	}
}    // namespace Rift
