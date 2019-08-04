// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"
#include "Identifier.h"
#include "VariableIdentifier.h"


struct FunctionIdentifier : public Identifier
{
	STRUCT(FunctionIdentifier, Identifier)

	P(Name, category);

	P(TArray<Name>, parameters);

	P(TArray<VariableIdentifier>, localVariables);
};
