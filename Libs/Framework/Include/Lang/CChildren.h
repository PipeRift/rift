// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <ECS.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CChildren : public Struct
	{
		STRUCT(CChildren, Struct)

		TArray<ECS::EntityId> children;
	};
}    // namespace Rift
