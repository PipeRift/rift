// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"


namespace rift::AST
{
	struct CDeclStatic : public CDeclaration
	{
		STRUCT(CDeclStatic, CDeclaration)
	};
}    // namespace rift::AST
