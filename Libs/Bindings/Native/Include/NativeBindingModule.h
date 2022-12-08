// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Module.h>
#include <Rift.h>


namespace rift
{
	class NativeBindingModule : public Module
	{
		CLASS(NativeBindingModule, Module)

	public:
		void Register(TPtr<Rift> rift) override
		{
			// Register module type
		}
	};

	void Sync();
}    // namespace rift
