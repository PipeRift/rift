// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Serialization.h"

#include <AST/Serialization.h>
#include <Misc/Checks.h>
#include <Profiler.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Types
{
	void InitFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category)
	{
		switch (category)
		{
			case TypeCategory::Class:
				ast.Emplace<CClassDecl>(id);
				break;
			case TypeCategory::Struct:
				ast.Emplace<CStructDecl>(id);
				break;
			case TypeCategory::FunctionLibrary:
				ast.Emplace<CFunctionLibraryDecl>(id);
				break;
		}
	}

	TypeCategory GetCategory(AST::Tree& ast, AST::Id id)
	{
		if (ast.Has<CStructDecl>(id))
		{
			return TypeCategory::Struct;
		}
		else if (ast.Has<CClassDecl>(id))
		{
			return TypeCategory::Class;
		}
		else if (ast.Has<CFunctionLibraryDecl>(id))
		{
			return TypeCategory::FunctionLibrary;
		}
		return TypeCategory::None;
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;
		Serl::JsonFormatWriter writer{};

		ASTWriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeRoot(id);

		data = writer.ToString();
	}
}    // namespace Rift::Types
