// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift::AST
{
	struct TypeRef : public Struct
	{
		STRUCT(TypeRef, Struct)

		PROP(AST::Id, id)
		AST::Id id;

		PROP(Guid, guid)
		Guid guid;
	};

	void Read(Serl::ReadContext& ct, TypeRef& value);
	void Write(Serl::WriteContext& ct, const TypeRef& value);
}    // namespace Rift::AST
