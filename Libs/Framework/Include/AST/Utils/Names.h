// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CNamespace.h"

#include <Pipe/ECS/Access.h>


namespace rift::Names
{
	Name GetName(TAccessRef<CNamespace> access, AST::Id id)
	{
		auto* identifier = access.TryGet<const CNamespace>(id);
		return identifier ? identifier->name : Name::None();
	}
};    // namespace rift::Names
