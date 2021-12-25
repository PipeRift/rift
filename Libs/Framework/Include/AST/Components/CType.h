// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift
{
	enum class TypeCategory : u8
	{
		None,
		Class,
		Struct,
		FunctionLibrary
	};

	struct CType : public Struct
	{
		STRUCT(CType, Struct)

		PROP(name)
		Name name;

		CType(Name name = {}) : name(name) {}
	};
}    // namespace Rift
