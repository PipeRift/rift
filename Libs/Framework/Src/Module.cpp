// Copyright 2015-2022 Piperift - All rights reserved

#include "Module.h"

#include "Rift.h"


namespace rift
{
	TPtr<Rift> Module::GetRift()
	{
		return GetOwner<Rift>();
	}
}    // namespace rift
