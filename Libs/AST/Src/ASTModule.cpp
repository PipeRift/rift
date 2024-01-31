// Copyright 2015-2023 Piperift - All rights reserved

#include "ASTModule.h"

#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Utils/ModuleUtils.h"
#include "AST/Utils/TypeUtils.h"


namespace rift
{
	const p::Tag ASTModule::structType = "Struct";
	const p::Tag ASTModule::classType  = "Class";
	const p::Tag ASTModule::staticType = "Static";


	void ASTModule::Load()
	{
		ast::RegisterFileType<ast::CDeclStruct>(
		    structType, {.displayName = "Struct", .hasVariables = true, .hasFunctions = false});
		ast::RegisterFileType<ast::CDeclClass>(
		    classType, {.displayName = "Class", .hasVariables = true, .hasFunctions = true});
		ast::RegisterFileType<ast::CDeclStatic>(
		    staticType, {.displayName = "Static", .hasVariables = true, .hasFunctions = true});
		ast::PreAllocPools<ast::CDeclStruct, ast::CDeclClass, ast::CDeclStatic>();

		ast::RegisterSerializedModulePools<ast::CModule>();
	}
}    // namespace rift
