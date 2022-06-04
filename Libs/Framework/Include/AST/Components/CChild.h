// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <Reflection/Struct.h>


namespace Rift
{
	using namespace Pipe;


	struct CChild : public Pipe::Struct
	{
		STRUCT(CChild, Pipe::Struct)

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
}    // namespace Rift
