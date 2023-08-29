// Copyright 2015-2023 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	static p::TMap<p::ClassType*, p::TOwnPtr<class Module>> gModules{};
	static p::TArray<View> gViews{};


	void EnableModule(p::ClassType* type)
	{
		Check(Module::GetStaticType()->IsParentOf(type));

		if (!gModules.Contains(type))
		{
			auto module = MakeOwned<Module>(type);
			module->DoLoad();
			gModules.Insert(type, Move(module));
		}
	}

	void DisableModule(p::ClassType* type)
	{
		gModules.Remove(type);
	}

	p::TPtr<Module> GetModule(p::ClassType* type)
	{
		if (auto* module = gModules.Find(type))
		{
			return *module;
		}
		return {};
	}

	void RegisterView(View view)
	{
		gViews.Add(Move(view));
	}
};    // namespace rift
