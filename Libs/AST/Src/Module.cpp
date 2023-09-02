// Copyright 2015-2023 Piperift - All rights reserved

#include "Module.h"

#include "Rift.h"


namespace rift
{
	void Module::DoLoad()
	{
		state = State::Loading;
		Load();
		state = State::Ready;
	}
}    // namespace rift