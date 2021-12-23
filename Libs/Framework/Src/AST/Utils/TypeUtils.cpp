// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Serialization.h"
#include "AST/Utils/Hierarchy.h"

#include <Misc/Checks.h>
#include <Profiler.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category)
	{
		switch (category)
		{
			case TypeCategory::Class: ast.Add<CClassDecl>(id); break;
			case TypeCategory::Struct: ast.Add<CStructDecl>(id); break;
			case TypeCategory::FunctionLibrary: ast.Add<CFunctionLibraryDecl>(id); break;
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

	AST::Id CreateType(AST::Tree& ast, TypeCategory type, Name name)
	{
		AST::Id id = ast.Create();
		InitTypeFromCategory(ast, id, type);
		if (!name.IsNone())
		{
			ast.Add<CIdentifier>(id, name);
		}
		return id;
	}

	AST::Id AddVariable(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CVariableDecl, CParent>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	AST::Id AddFunction(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CFunctionDecl, CParent>(id);
		ast.Add<CStatementOutputs>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;
		Serl::JsonFormatWriter writer{};

		AST::WriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeRoot(id);

		if (auto* identifier = ast.TryGet<CIdentifier>(id))
		{
			ct.Next("name", identifier->name);
		}

		data = writer.ToString();
	}

	bool IsClass(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CClassDecl>(typeId);
	}

	bool IsStruct(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CStructDecl>(typeId);
	}

	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CFunctionLibraryDecl>(typeId);
	}

	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CClassDecl, CStructDecl>(typeId);
	}

	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CClassDecl, CFunctionLibraryDecl>(typeId);
	}
}    // namespace Rift::Types
