// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <PCore/Array.h>
#include <PRefl/Struct.h>


namespace rift
{
	using namespace p;

	struct CParent : public p::Struct
	{
		STRUCT(CParent, p::Struct)

		PROP(children)
		TArray<AST::Id> children;
	};

	static void Read(Reader& ct, CParent& val)
	{
		ct.Serialize(val.children);
	}
	static void Write(Writer& ct, const CParent& val)
	{
		ct.Serialize(val.children);
	}
}    // namespace rift
