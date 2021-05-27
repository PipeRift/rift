// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>
#include <Strings/Name.h>


namespace Rift
{
	struct CIdentifier : public Struct
	{
		STRUCT(CIdentifier, Struct)

		PROP(Name, name);
		Name name;


		CIdentifier() = default;
		CIdentifier(Name name) : name(name) {}
	};
}    // namespace Rift
