// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "PipePlatform.h"
#include "Rift.h"

#include <Pipe/Memory/OwnPtr.h>
#include <PipeReflect.h>


namespace rift
{
	using namespace p;


	class Module : public Object
	{
		using Super = Object;
		P_CLASS(Module)

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
			P_EnsureMsg(state == State::Uninitialized,
			    "Should not add dependencies outside of the constructor");

			EnableModule<ModuleType>();
		}
	};
}    // namespace rift
