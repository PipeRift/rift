// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/Class.h>


namespace rift
{
	using namespace p;

	class Plugin : public Class
	{
		CLASS(Plugin, Class)

	public:
		virtual void Register(TPtr<struct Rift> rift) = 0;
	};
}    // namespace rift
