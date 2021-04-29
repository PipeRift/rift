// Copyright 2015-2020 Piperift - All rights reserved

#include "Lang/AST.h"
#include "Lang/CChildren.h"
#include "Lang/Declarations/ClassDecl.h"
#include "Lang/Declarations/StructDecl.h"
#include "Lang/Identifiers/Identifier.h"

namespace Rift
{
	AST::Id AST::CreateClass(Name name)
	{
		Id classId = registry.Create();
		AddComponent<CIdentifier>(classId, name);
		AddComponent<CClassDecl>(classId);
		AddComponent<CChildren>(classId);
		return classId;
	}

	AST::Id AST::CreateStruct(Name name)
	{
		Id classId = registry.Create();
		AddComponent<CIdentifier>(classId, name);
		AddComponent<CStructDecl>(classId);
		AddComponent<CChildren>(classId);
		return classId;
	}
}    // namespace Rift
