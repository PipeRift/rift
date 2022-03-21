// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Misc/Guid.h>
#include <Types/Struct.h>


namespace Rift
{
	enum class Type : u8
	{
		None            = 0,
		Class           = 1 << 0,
		Struct          = 1 << 1,
		FunctionLibrary = 1 << 2
	};


	struct CType : public Struct
	{
		STRUCT(CType, Struct)

		PROP(name)
		Name name;

		PROP(view)
		Name view;

		CType(Name name = {}) : name(name) {}
	};
}    // namespace Rift
