// Copyright 2015-2024 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	static p::TMap<p::TypeId, p::TOwnPtr<class Module>> gModules{};
	static p::TArray<View> gViews{};


	void EnableModule(p::TypeId type)
	{
		P_Check(IsTypeParentOf(GetTypeId<Module>(), type));

		if (!gModules.Contains(type))
		{
			auto module = MakeOwned<Module>(type);
			module->DoLoad();
			gModules.Insert(type, Move(module));
		}
	}

	void DisableModule(p::TypeId type)
	{
		gModules.Remove(type);
	}

	p::TPtr<Module> GetModule(p::TypeId type)
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
