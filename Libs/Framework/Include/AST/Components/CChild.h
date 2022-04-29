// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Types.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CChild : public Struct
	{
		STRUCT(CChild, Struct)

		PROP(parent)
		AST::Id parent = AST::NoId;


		CChild() = default;
		CChild(AST::Id parent) : parent(parent) {}
	};
	static void Read(Serl::ReadContext& ct, CChild& val)
	{
		ct.Serialize(val.parent);
	}
	static void Write(Serl::WriteContext& ct, const CChild& val)
	{
		ct.Serialize(val.parent);
	}
}    // namespace Rift
