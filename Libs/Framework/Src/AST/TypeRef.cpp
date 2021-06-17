// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/TypeRef.h"

#include <Serialization/Contexts.h>


namespace Rift::AST
{
	void Read(Serl::ReadContext& ct, TypeRef& value)
	{
		ct.Serialize(value.guid);
	}

	void Write(Serl::WriteContext& ct, const TypeRef& value)
	{
		ct.Serialize(value.guid);
	}
}    // namespace Rift::AST
