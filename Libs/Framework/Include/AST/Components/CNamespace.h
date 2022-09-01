// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Name.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
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

	inline void Read(p::Reader& r, CNamespace& val)
	{
		r.Serialize(val.name);
	}
	inline void Write(p::Writer& w, const CNamespace& val)
	{
		w.Serialize(val.name);
	}
}    // namespace rift::AST
