// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "AST/Components/CIdentifier.h"


namespace Rift::Names
{
	Name GetName(TAccessRef<CIdentifier> access, AST::Id id)
	{
		auto* identifier = access.TryGet<const CIdentifier>(id);
		return identifier ? identifier->name : Name::None();
	}
};    // namespace Rift::Names
