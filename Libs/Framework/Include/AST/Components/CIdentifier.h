// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Name.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	using namespace p;


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
}    // namespace rift
