// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Assets/AssetData.h>
#include <CoreObject.h>
#include <Reflection/TypeFlags.h>


namespace Rift
{
	class TypeSystem : public Object
	{
		CLASS(TypeSystem, Object)

	public:
		static void Tick();
	};
}    // namespace Rift
