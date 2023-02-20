// Copyright 2015-2023 Piperift - All rights reserved

#include "FrameworkModule.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclStatic.h"
#include "AST/Components/CDeclStruct.h"

#include <Pipe/Memory/NewDelete.h>


P_OVERRIDE_NEW_DELETE

namespace rift
{
	const p::Tag FrameworkModule::structType = "Struct";
	const p::Tag FrameworkModule::classType  = "Class";
	const p::Tag FrameworkModule::staticType = "Static";


	void FrameworkModule::Load()
	{
		RegisterRiftType<AST::CDeclStruct>(
		    structType, {.displayName = "Struct", .hasVariables = true, .hasFunctions = false});
		RegisterRiftType<AST::CDeclClass>(
		    classType, {.displayName = "Class", .hasVariables = true, .hasFunctions = true});
		RegisterRiftType<AST::CDeclStatic>(
		    staticType, {.displayName = "Static", .hasVariables = true, .hasFunctions = true});
	}
}    // namespace rift
