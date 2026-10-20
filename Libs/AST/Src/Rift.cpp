// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Rift.h"

#include "Module.h"

#include <PipeStrings.h>
#include <PipeTime.h>

#include <iostream>


namespace rift
{
	static p::TMap<p::TypeId, p::TOwnPtr<class Module>> gModules{};
	static p::TArray<View> gViews{};


	p::Logger MakeLogger()
	{
		auto log = [](StringView level, StringView msg)
		{
			String text;
			p::DateTime now = p::DateTime::Now();
			now.ToString("[%Y/%m/%d %H:%M:%S]", text);
			p::FormatTo(text, "[{}] {}\n", level, msg);
			std::cout << text;
		};

		return p::Logger{
		    .infoCallback =
		        [log](StringView msg)
		{
			log("Info", msg);
		},
		    .warningCallback =
		        [log](StringView msg)
		{
			log("Warning", msg);
		},
		    .errorCallback =
		        [log](StringView msg)
		{
			log("Error", msg);
		},
		};
	}


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

	void DisableAllModules()
	{
		gModules.Clear();
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
