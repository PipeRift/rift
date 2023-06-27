// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CDeclType.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "View.h"

#include <Pipe/Core/Tag.h>
#include <Pipe/Core/View.h>
#include <Pipe/PipeECS.h>
#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	void EnableModule(p::ClassType* type);
	void DisableModule(p::ClassType* type);
	p::TPtr<class Module> GetModule(p::ClassType* type);


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
		return GetModule(T::GetStaticType()).template Cast<T>();
	}

	void RegisterView(View view);
}    // namespace rift
