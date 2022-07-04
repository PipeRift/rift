// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CChild.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "AST/Utils/Hierarchy.h"

#include <Pipe/ECS/Access.h>


namespace rift::AST
{
	Name GetName(TAccessRef<CNamespace, CChild> access, Id id);
	p::String&& GetFullName(
	    TAccessRef<CNamespace, CChild> access, Id id, bool relativeNamespace = false);

	void GetNamespace(TAccessRef<CNamespace, CChild> access, Id id, p::String& ns);
	void GetLocalNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id, p::String& ns);

	bool ExistsNamespace(p::String ns);
	bool IsUniqueNamespace(p::String ns);
}    // namespace rift::AST
