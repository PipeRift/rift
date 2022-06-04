// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Name.h>
#include <Reflection/Struct.h>


namespace Rift
{
	using namespace Pipe;


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
