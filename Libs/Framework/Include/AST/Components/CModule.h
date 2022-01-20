// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Types/Struct.h>


namespace Rift
{
	enum class ModuleTarget : u8
	{
		Executable,
		Shared,
		Static
	};
	// TODO: Simplify enum reflection so that ENUM() is not needed
	ENUM(ModuleTarget)

	struct CModule : public Struct
	{
		STRUCT(CModule, Struct)

		PROP(target)
		ModuleTarget target = ModuleTarget::Executable;


		CModule() {}
	};
}    // namespace Rift
