// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "AST/Components/Declarations.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "View.h"

#include <Pipe/Core/Tag.h>
#include <PipeECS.h>
#include <PipeReflect.h>


namespace rift
{
	void EnableModule(p::TypeId type);
	void DisableModule(p::TypeId type);
	p::TPtr<class Module> GetModule(p::TypeId type);


	template<typename T>
	void EnableModule()
	{
		EnableModule(p::GetTypeId<T>());
	}
	template<typename T>
	void DisableModule()
	{
		DisableModule(p::GetTypeId<T>());
	}
	template<typename T>
	p::TPtr<T> GetModule()
	{
		return Cast<T>(GetModule(p::GetTypeId<T>()));
	}

	void RegisterView(View view);
}    // namespace rift
