// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <Core/Array.h>
#include <Reflection/Struct.h>


namespace rift
{
	using namespace p;

	struct CParent : public p::Struct
	{
		STRUCT(CParent, p::Struct)

		PROP(children)
		TArray<AST::Id> children;
	};

	static void Read(ReadContext& ct, CParent& val)
	{
		ct.Serialize(val.children);
	}
	static void Write(WriteContext& ct, const CParent& val)
	{
		ct.Serialize(val.children);
	}
}    // namespace rift
