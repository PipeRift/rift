// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declaration.h"


namespace Rift
{
	struct VariableDecl : public Declaration
	{
		STRUCT(VariableDecl, Declaration)

		// TypeRef type;

		Name name;
	};
}    // namespace Rift
