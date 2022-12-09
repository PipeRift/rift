// Copyright 2015-2022 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	void Rift::EnableModule(p::ClassType* type)
	{
		Check(Module::GetStaticType()->IsParentOf(type));

		if (!modules.Contains(type))
		{
			modules.Insert(type, MakeOwned<Module>(type, Self()));
		}
	}

	void Rift::DisableModule(p::ClassType* type)
	{
		modules.Remove(type);
	}

	p::TPtr<Module> Rift::GetModule(p::ClassType* type)
	{
		if (auto* module = modules.Find(type))
		{
			return *module;
		}
		return {};
	}
};    // namespace rift
