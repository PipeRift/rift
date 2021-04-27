// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>


namespace Rift
{
	struct CIdentifier : public Struct
	{
		STRUCT(CIdentifier, Struct)

		PROP(Name, name);
		Name name;
	};
}    // namespace Rift
