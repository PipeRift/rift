// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreStruct.h"


class Identifier : public Struct
{
	STRUCT(Identifier, Struct)

	P(Name, name);
};
