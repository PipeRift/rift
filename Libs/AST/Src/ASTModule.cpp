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
		AST::RegisterFileType<AST::CDeclStruct>(
		    structType, {.displayName = "Struct", .hasVariables = true, .hasFunctions = false});
		AST::RegisterFileType<AST::CDeclClass>(
		    classType, {.displayName = "Class", .hasVariables = true, .hasFunctions = true});
		AST::RegisterFileType<AST::CDeclStatic>(
		    staticType, {.displayName = "Static", .hasVariables = true, .hasFunctions = true});
		AST::PreAllocPools<AST::CDeclStruct, AST::CDeclClass, AST::CDeclStatic>();

		AST::RegisterSerializedModulePools<AST::CModule>();
	}
}    // namespace rift
