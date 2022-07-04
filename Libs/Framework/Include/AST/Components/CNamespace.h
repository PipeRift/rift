// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Name.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CNamespace : public p::Struct
	{
		STRUCT(CNamespace, p::Struct)

		PROP(name);
		p::Name name;


		CNamespace() = default;
		CNamespace(p::Name name) : name(name) {}

		bool operator==(const CNamespace& other) const
		{
			return name == other.name;
		}
		bool operator==(p::Name other) const
		{
			return name == other;
		}
	};
}    // namespace rift