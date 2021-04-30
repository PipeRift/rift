// Copyright 2015-2020 Piperift - All rights reserved

#include "Lang/AST.h"
#include "Lang/CChildren.h"
#include "Lang/Declarations/CClassDecl.h"
#include "Lang/Declarations/CStructDecl.h"
#include "Lang/Identifiers/CIdentifier.h"

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
