// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Memory/OwnPtr.h>
#include <Types/Class.h>


namespace Rift
{
	class Plugin : public Class
	{
		CLASS(Plugin, Class)

	public:
		virtual void Register(TPtr<struct RiftContext> context) = 0;
	};
}    // namespace Rift