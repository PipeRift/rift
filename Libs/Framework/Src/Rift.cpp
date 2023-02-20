// Copyright 2015-2023 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	static p::TMap<p::ClassType*, p::TOwnPtr<class Module>> modules{};
	static p::TArray<View> views{};


	void EnableModule(p::ClassType* type)
	{
		Check(Module::GetStaticType()->IsParentOf(type));

		if (!modules.Contains(type))
		{
			auto module = MakeOwned<Module>(type);
			module->DoLoad();
			modules.Insert(type, Move(module));
		}
	}

	void DisableModule(p::ClassType* type)
	{
		modules.Remove(type);
	}

	p::TPtr<Module> GetModule(p::ClassType* type)
	{
		if (auto* module = modules.Find(type))
		{
			return *module;
		}
		return {};
	}

	void RegisterView(View view)
	{
		views.Add(Move(view));
	}
};    // namespace rift
