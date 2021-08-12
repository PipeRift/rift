// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <Containers/Map.h>
#include <Misc/Guid.h>
#include <Object/Struct.h>


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

		using TypeMap = TMap<Guid, AST::Id>;

		PROP(isMain, Prop_Transient)
		bool isMain = false;

		PROP(target)
		ModuleTarget target = ModuleTarget::Executable;


		CModule(bool isMain) : isMain{isMain} {}
	};
}    // namespace Rift
