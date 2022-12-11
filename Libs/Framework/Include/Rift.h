// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "View.h"

#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	template<typename T>
	void EnableModule()
	{
		EnableModule(T::GetStaticType());
	}
	template<typename T>
	void DisableModule()
	{
		DisableModule(T::GetStaticType());
	}
	template<typename T>
	p::TPtr<T> GetModule()
	{
		return GetModule(T::GetStaticType()).Cast<T>();
	}

	void EnableModule(p::ClassType* type);
	void DisableModule(p::ClassType* type);
	p::TPtr<class Module> GetModule(p::ClassType* type);

	void RegisterView(View view);
}    // namespace rift
