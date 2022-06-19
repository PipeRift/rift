// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Guid.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	using namespace p;


	enum class RiftType : u8
	{
		None              = 0,
		Class             = 1 << 0,
		Struct            = 1 << 1,
		FunctionLibrary   = 1 << 2,
		FunctionInterface = 1 << 3
	};
	PIPE_DEFINE_FLAG_OPERATORS(RiftType)


	struct CType : public Struct
	{
		STRUCT(CType, Struct)

		PROP(name)
		Name name;

		PROP(view)
		Name view;

		CType(Name name = {}) : name(name) {}
	};
}    // namespace rift
