// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>
#include <Strings/Name.h>


namespace Rift
{
	struct CIdentifier : public Struct
	{
		STRUCT(CIdentifier, Struct)

		PROP(name);
		Name name;


		CIdentifier() = default;
		CIdentifier(Name name) : name(name) {}

		bool operator==(const CIdentifier& other) const
		{
			return name == other.name;
		}
		bool operator==(Name other) const
		{
			return name == other;
		}
	};
}    // namespace Rift
