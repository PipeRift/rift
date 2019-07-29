// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"
#include "Container.h"


class ClassContainer : public Container
{
    STRUCT(ClassContainer, Container)

    PROP(TArray<VariableIdentifier>, variables)
    TArray<VariableIdentifier> variables;

    PROP(TArray<FunctionIdentifier>, functions)
    TArray<FunctionIdentifier> functions;
};
