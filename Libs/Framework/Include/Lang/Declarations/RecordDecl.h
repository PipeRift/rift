// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Lang/Declaration.h"
#include "Lang/Declarations/VariableDecl.h"


namespace Rift
{
	struct RecordDecl : public Declaration
	{
		STRUCT(RecordDecl, Declaration)

		TArray<TOwnPtr<VariableDecl>> variables;
	};
}    // namespace Rift
