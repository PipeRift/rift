// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/Class.h>


namespace rift
{
	using namespace p;

	class Module : public Class
	{
		CLASS(Module, Class)

	public:
		virtual void Register(TPtr<struct Rift> rift) = 0;

		template<typename ModuleType>
		void AddDependency()
		{
			GetRift()->EnableModule<ModuleType>();
		}

		TPtr<struct Rift> GetRift();
	};
}    // namespace rift
