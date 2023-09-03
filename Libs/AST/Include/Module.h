// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Rift.h"

#include <Pipe/Memory/OwnPtr.h>
#include <Pipe/Reflect/Class.h>


namespace rift
{
	using namespace p;


	class Module : public Class
	{
		P_CLASS(Module, Class)

		enum class State : u8
		{
			Uninitialized,
			Initialized,
			Loading,
			Ready
		};

		State state = State::Uninitialized;


	public:
		void DoLoad();

		bool IsLoading() const
		{
			return state == State::Loading;
		}

	protected:
		virtual void Load() {}

		template<typename ModuleType>
		void AddDependency()
		{
			EnsureMsg(state == State::Uninitialized,
			    "Should not add dependencies outside of the constructor");

			EnableModule<ModuleType>();
		}
	};
}    // namespace rift
