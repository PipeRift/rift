// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Systems/LoadSystem.h"

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CType.h"
#include "AST/Serialization.h"
#include "AST/Tree.h"
#include "AST/Uniques/CLoadQueueUnique.h"
#include "AST/Uniques/CStringLoadUnique.h"

#include <Files/Files.h>
#include <Serialization/Formats/JsonFormat.h>


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

		stringLoad.entities.Empty();

		stringLoad.entities = Move(loadQueue.pendingAsyncLoad);

		stringLoad.paths.Resize(stringLoad.entities.Size());
		auto filesView = ast.MakeView<CFileRef>();
		for (u32 i = 0; i < stringLoad.entities.Size(); ++i)
		{
			if (auto* file = filesView.TryGet<CFileRef>(stringLoad.entities[i]))
			{
				stringLoad.paths[i] = file->path;
			}
			else
			{
				stringLoad.paths[i] = Path{};
			}
		}

		LoadStrings(stringLoad);
	}

	void LoadStrings(CStringLoadUnique& stringLoad)
	{
		auto& paths   = stringLoad.paths;
		auto& strings = stringLoad.strings;
		strings.Resize(paths.Size());

		for (u32 i = 0; i < paths.Size(); ++i)
		{
			String& str = strings[i];
			if (!Files::LoadStringFile(paths[i], str, 4))
			{
				str = {};
				Log::Error("File ({}) could not be loaded from disk", Paths::ToString(paths[i]));
				continue;
			}
		}
		paths.Empty();
	}

	void Deserialize(AST::Tree& ast, CStringLoadUnique& stringLoad)
	{
		auto& entities = stringLoad.entities;
		auto& strings  = stringLoad.strings;
		Check(entities.Size() == strings.Size());

		for (u32 i = 0; i < strings.Size(); ++i)
		{
			String& str = strings[i];
			if (str.empty())
			{
				continue;
			}
			AST::Id entity = entities[i];

			Serl::JsonFormatReader reader{str};
			ASTReadContext ct{reader, ast};

			ct.BeginObject();
			TypeCategory category;
			ct.Next("type", category);
			switch (category)
			{
				case TypeCategory::Class:
					ast.Emplace<CClassDecl>(entity);
					break;
				case TypeCategory::Struct:
					ast.Emplace<CStructDecl>(entity);
					break;
				case TypeCategory::FunctionLibrary:
					ast.Emplace<CFunctionLibraryDecl>(entity);
					break;
			}

			ct.SerializeRoot(entity);
		}
	}
}    // namespace Rift::LoadSystem
