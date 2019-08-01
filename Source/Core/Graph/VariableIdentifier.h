// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"
#include "Identifier.h"


class VariableIdentifier : public Identifier
{
	STRUCT(VariableIdentifier, Identifier)

	P(String, defaultValue);
};
