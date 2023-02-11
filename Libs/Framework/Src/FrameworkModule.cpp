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
		RegisterFileType<AST::CDeclStruct>(structType, "Struct");
		RegisterFileType<AST::CDeclClass>(classType, "Class");
		RegisterFileType<AST::CDeclStatic>(staticType, "Static");
	}
}    // namespace rift
