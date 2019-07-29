// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"
#include "Identifier.h"
#include "VariableIdentifier.h"


class FunctionIdentifier : public Identifier
{
    STRUCT(FunctionIdentifier, Identifier)

    PROP(TArray<VariableIdentifier>, parameters)
    TArray<VariableIdentifier> parameters;

    PROP(TArray<VariableIdentifier>, localVariables)
    TArray<VariableIdentifier> localVariables;
};
