// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <Reflection/Struct.h>


namespace rift
{
	using namespace p;


	struct CChild : public p::Struct
	{
		STRUCT(CChild, p::Struct)

		PROP(parent)
		AST::Id parent = AST::NoId;


		CChild() = default;
		CChild(AST::Id parent) : parent(parent) {}
	};

	static void Read(ReadContext& ct, CChild& val)
	{
		ct.Serialize(val.parent);
	}
	static void Write(WriteContext& ct, const CChild& val)
	{
		ct.Serialize(val.parent);
	}
}    // namespace rift
