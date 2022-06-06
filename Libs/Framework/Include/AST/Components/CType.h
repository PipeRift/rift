// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <PCore/Guid.h>
#include <PRefl/Struct.h>


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
	DEFINE_FLAG_OPERATORS(RiftType)


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
