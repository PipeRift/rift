// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <Containers/Array.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CParent : public Struct
	{
		STRUCT(CParent, Struct)

		PROP(children)
		TArray<AST::Id> children;
	};
	static void Read(Serl::ReadContext& ct, CParent& val)
	{
		ct.Serialize(val.children);
	}
	static void Write(Serl::WriteContext& ct, const CParent& val)
	{
		ct.Serialize(val.children);
	}
}    // namespace Rift
