// Copyright 2015-2022 Piperift - All rights reserved

#include "CExprOutputs.h"

#include "CExprInputs.h"


namespace Rift
{
	InputId::InputId(TAccessRef<CExprInputs, CChild> access, AST::Id pinId) : pinId(pinId)
	{
		nodeId = pinId;
		// If node is not the pin itself, it must be the parent
		if (!IsNone(nodeId) && !access.Has<CExprInputs>(nodeId))
		{
			nodeId = AST::Hierarchy::GetParent(access, pinId);
		}
	}
}    // namespace Rift
