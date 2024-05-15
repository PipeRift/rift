// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "AST/Components/Declarations.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "View.h"

#include <Pipe/Core/Tag.h>
#include <PipeECS.h>


namespace rift
{
	void EnableModule(p::TypeId type);
	void DisableModule(p::TypeId type);
	p::TPtr<class Module> GetModule(p::TypeId type);


	template<typename T>
	void EnableModule()
	{
		EnableModule(GetTypeId<T>());
	}
	template<typename T>
	void DisableModule()
	{
		DisableModule(GetTypeId<T>());
	}
	template<typename T>
	p::TPtr<T> GetModule()
	{
		return GetModule(GetTypeId<T>()).template Cast<T>();
	}

	void RegisterView(View view);
}    // namespace rift
