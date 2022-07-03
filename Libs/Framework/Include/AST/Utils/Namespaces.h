// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CIdentifier.h"
#include "AST/Id.h"
#include "AST/Utils/Hierarchy.h"

#include <Pipe/ECS/Access.h>


namespace rift::AST
{
	bool GetRelativeNamespace(
	    TAccessRef<CIdentifier, CChild> access, Id id, p::String& ns, Id relativeParentId = NoId);
	void GetFullNamespace(TAccessRef<CIdentifier, CChild> access, Id id, p::String& ns);
	bool GetName(TAccessRef<CIdentifier, CChild> access, Id id, p::String& name,
	    bool includeNamespace = false, bool relativeNamespace = false);

	bool ExistsNamespace(p::String ns);
	bool IsUniqueNamespace(p::String ns);
}    // namespace rift::AST
