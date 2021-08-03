// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"
#include "Assets/ModuleAsset.h"

#include <Containers/Map.h>
#include <Misc/Guid.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CModule : public Struct
	{
		STRUCT(CModule, Struct)

		using TypeMap = TMap<Guid, AST::Id>;

		// TODO: Reflect Maps
		// PROP(TypeMap, types, Transient)
		TypeMap types;

		PROP(bool, isMain, Transient)
		bool isMain = false;


		CModule(bool isMain) : isMain{isMain} {}
	};
}    // namespace Rift
