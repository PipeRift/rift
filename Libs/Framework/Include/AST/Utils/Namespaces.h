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
	void GetNamespace(TAccessRef<CNamespace, CChild> access, Id id, p::String& ns);
	void GetLocalNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id, p::String& ns);

	Name GetName(TAccessRef<CNamespace> access, Id id);
	Name GetNameChecked(TAccessRef<CNamespace> access, Id id);
	p::String GetFullName(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, bool localNamespace = false);
	p::String GetFullNameChecked(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, bool localNamespace = false);

	bool ExistsNamespace(p::String ns);
	bool IsUniqueNamespace(p::String ns);
}    // namespace rift::AST
