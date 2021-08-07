// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Tree.h"
#include "AST/Uniques/CLoadQueueUnique.h"
#include "AST/Uniques/CStringLoadUnique.h"

#include <Files/Files.h>


namespace Rift::LoadSystem
{
	void LoadStrings(CStringLoadUnique& stringLoad);
	void Deserialize(AST::Tree& ast, CStringLoadUnique& stringLoad);

	void Init(AST::Tree& ast)
	{
		ast.SetUnique<CLoadQueueUnique>();
		ast.SetUnique<CStringLoadUnique>();
	}

	void Run(AST::Tree& ast)
	{
		auto& loadQueue  = ast.GetUnique<CLoadQueueUnique>();
		auto& stringLoad = ast.GetUnique<CStringLoadUnique>();


		// Dump loaded data
		Deserialize(ast, stringLoad);

		stringLoad.entities.Append(loadQueue.pendingAsyncLoad);
		stringLoad.paths.Reserve(stringLoad.entities.Size());
		// for each file -> get path
	}

	void LoadStrings(CStringLoadUnique& stringLoad)
	{
		auto& paths   = stringLoad.paths;
		auto& strings = stringLoad.strings;
		Check(paths.Size() == strings.Size());

		for (u32 i = 0; i < paths.Size(); ++i)
		{
			if (!Files::LoadStringFile(paths[i], strings[i], 4))
			{
				Log::Error("File ({}) could not be loaded from disk", Paths::ToString(paths[i]));
				continue;
			}
		}
	}

	void Deserialize(AST::Tree& ast, CStringLoadUnique& stringLoad)
	{
		auto& entities = stringLoad.entities;
		auto& strings  = stringLoad.strings;
		Check(entities.Size() == strings.Size());

		for (u32 i = 0; i < strings.Size(); ++i)
		{
			// Convert strings into components
		}
	}
}    // namespace Rift::LoadSystem
