// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <AST/Access.h>
#include <AST/Components/CChild.h>
#include <AST/Components/CFileRef.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CType.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, Type category);
	Type GetCategory(AST::Tree& ast, AST::Id id);

	AST::Id CreateType(AST::Tree& ast, Type type, Name name = Name::None(), const Path& path = {});

	AST::Id AddVariable(AST::TypeRef type, Name name);
	AST::Id AddFunction(AST::TypeRef type, Name name);

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access,
	    TSpan<AST::Id> types, bool removeFromDisk = false);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);
	void Deserialize(AST::Tree& ast, AST::Id id, const String& data);

	AST::Id FindTypeByPath(AST::Tree& ast, const Path& path);
	bool IsClass(const AST::Tree& ast, AST::Id typeId);
	bool IsStruct(const AST::Tree& ast, AST::Id typeId);
	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId);
	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId);
	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId);
}    // namespace Rift::Types
