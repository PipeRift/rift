// Copyright 2015-2026 Piperift. All Rights Reserved.

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