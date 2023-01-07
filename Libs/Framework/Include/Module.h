// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Rift.h"

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/Class.h>


namespace rift
{
	using namespace p;

	class Module : public Class
	{
		CLASS(Module, Class)

	public:
		virtual void Register() {}

		template<typename ModuleType>
		void AddDependency()
		{
			EnableModule<ModuleType>();
		}
	};
}    // namespace rift
