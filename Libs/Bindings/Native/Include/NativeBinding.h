// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Plugin.h>
#include <Rift.h>


namespace rift
{
	class NativeBindingPlugin : public Plugin
	{
		CLASS(NativeBindingPlugin, Plugin)

	public:
		void Register(TPtr<Rift> rift) override
		{
			// Register module type
		}
	};

	void Sync();
}    // namespace rift
