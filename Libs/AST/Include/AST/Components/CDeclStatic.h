// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclaration.h"


namespace rift::AST
{
	struct CDeclStatic : public CDeclaration
	{
		P_STRUCT(CDeclStatic, CDeclaration)
	};
}    // namespace rift::AST
