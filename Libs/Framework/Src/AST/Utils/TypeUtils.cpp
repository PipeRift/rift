// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CCompoundStmt.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Linkage.h"
#include "AST/Serialization.h"
#include "AST/Utils/TypeUtils.h"

#include <Misc/Checks.h>
#include <Profiler.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, TypeCategory category)
	{
		switch (category)
		{
			case TypeCategory::Class: ast.Emplace<CClassDecl>(id); break;
			case TypeCategory::Struct: ast.Emplace<CStructDecl>(id); break;
			case TypeCategory::FunctionLibrary: ast.Emplace<CFunctionLibraryDecl>(id); break;
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

	AST::Id CreateClass(AST::Tree& ast, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CStructDecl, CParent>(id);
		return id;
	}

	AST::Id CreateStruct(AST::Tree& ast, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CStructDecl, CParent>(id);
		return id;
	}

	AST::Id CreateVariable(AST::Tree& ast, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CVariableDecl, CParent>(id);
		return id;
	}

	AST::Id CreateFunction(AST::Tree& ast, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CFunctionDecl, CParent>(id);

		AST::Id compoundId = ast.Create();
		ast.Add<CCompoundStmt>(id);
		AST::Link(ast, id, compoundId);
		return id;
	}

	AST::Id CreateLiteral(AST::Tree& ast, AST::Id typeId, AST::Id parentId)
	{
		const AST::Id literalId = ast.Create();

		bool created        = false;
		const auto& natives = ast.GetNativeTypes();
		if (typeId == natives.boolId)
		{
			ast.Add<CBoolLiteral>(literalId);
			created = true;
		}
		else if (typeId == natives.floatId)
		{
			ast.Add<CFloatLiteral>(literalId);
			created = true;
		}
		else if (typeId == natives.stringId)
		{
			ast.Add<CStringLiteral>(literalId);
			created = true;
		}

		if (!created)
		{
			ast.Destroy(literalId);
			return AST::NoId;
		}
		AST::Link(ast, parentId, literalId);
		return literalId;
	}

	AST::Id CreateCall(AST::Tree& ast, AST::Id functionId, AST::Id parentId)
	{
		const AST::Id callId = ast.Create();

		// TODO: Reference the function
		auto& expr      = ast.Add<CCallExpr>(callId);
		expr.functionId = functionId;

		AST::Link(ast, parentId, callId);
		return callId;
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;
		Serl::JsonFormatWriter writer{};

		ASTWriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeRoot(id);

		if (auto* identifier = ast.TryGet<CIdentifier>(id))
		{
			ct.Next("name", identifier->name);
		}

		data = writer.ToString();
	}
}    // namespace Rift::Types
