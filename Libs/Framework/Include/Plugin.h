// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Memory/OwnPtr.h>
#include <PRefl/Class.h>


namespace rift
{
	using namespace p;

	class Plugin : public Class
	{
		CLASS(Plugin, Class)

	public:
		virtual void Register(TPtr<struct RiftContext> context) = 0;
	};
}    // namespace rift
