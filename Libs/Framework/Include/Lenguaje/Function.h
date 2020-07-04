// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <CoreEngine.h>


struct Function : public Struct
{
	STRUCT(Function, Struct)

	P(String, name);
};
