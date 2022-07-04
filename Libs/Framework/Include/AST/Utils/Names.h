// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CNamespace.h"

#include <Pipe/ECS/Access.h>


namespace rift::Names
{
	Name GetName(TAccessRef<CNamespace> access, AST::Id id)
	{
		auto* ns = access.TryGet<const CNamespace>(id);
		return ns ? ns->name : Name::None();
	}
};    // namespace rift::Names
