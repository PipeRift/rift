// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "AST/Components/CIdentifier.h"


namespace Rift::Names
{
	Name GetName(TAccessRef<CIdentifier> access, AST::Id id)
	{
		return access.Get<const CIdentifier>(id).name;
	}
};    // namespace Rift::Names
