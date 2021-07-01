// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Linkage.h"
#include "AST/Tree.h"
#include "Compiler/Cpp/CodeGen.h"
#include "Compiler/Cpp/CppBackend.h"
#include "Compiler/CompilerContext.h"

#include <Files/Files.h>
#include <Strings/String.h>


namespace Rift::Compiler::Cpp
{
	void Spacing(String& code)
	{
		Strings::FormatTo(code, "\n");
	}

	void Comment(String& code, StringView text)
	{
		Strings::FormatTo(code, "// {}\n", text);
	}

	void AddInclude(String& code, StringView name)
	{
		Strings::FormatTo(code, "#include <{}>\n", name);
	}

	void ForwardDeclareStruct(String& code, StringView name)
	{
		Strings::FormatTo(code, "struct {};\n", name);
	}
	void DeclareStruct(String& code, StringView name, StringView super = {},
	    TFunction<void(String&)> buildContent = {})
	{
		String innerCode;
		if (buildContent)
		{
			buildContent(innerCode);
		}

		if (!super.empty())
		{
			Strings::FormatTo(code,
			    "struct {} : public {}\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    name, super, innerCode);
		}
		else
		{
			Strings::FormatTo(code,
			    "struct {}\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    name, innerCode);
		}
	}

	void AddVariable(String& code, StringView type, StringView name, StringView defaultValue)
	{
		if (defaultValue.empty())
		{
			Strings::FormatTo(code, "{} {};\n", type, name);
		}
		else
		{
			Strings::FormatTo(code, "{} {} {{ {} }};\n", type, name, defaultValue);
		}
	}

	void DeclareFunction(String& code, StringView name, StringView owner = {})
	{
		if (owner.empty())
		{
			Strings::FormatTo(code, "void {}();\n", name);
		}
		else
		{
			Strings::FormatTo(code, "void {}_{}({}& self);\n", owner, name, owner);
		}
	}

	void DefineFunction(String& code, StringView name, StringView owner = {},
	    TFunction<void(String&)> buildContent = {})
	{
		String innerCode;
		if (buildContent)
		{
			buildContent(innerCode);
		}

		if (owner.empty())
		{
			Strings::FormatTo(code,
			    "inline void {}()\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    name, innerCode);
		}
		else
		{
			Strings::FormatTo(code,
			    "inline void {}_{}({}& self)\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    owner, name, owner, innerCode);
		}
	}

	void ForwardDeclareTypes(String& code, const AST::Tree& ast)
	{
		auto structs = ast.MakeView<const CIdentifier, const CStructDecl>();
		for (AST::Id entity : structs)
		{
			const auto& identifier = structs.Get<const CIdentifier>(entity);
			ForwardDeclareStruct(code, identifier.name.ToString());
		}

		auto classes = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : classes)
		{
			const auto& identifier = classes.Get<const CIdentifier>(entity);
			ForwardDeclareStruct(code, identifier.name.ToString());
		}
	}

	void AddTypeVariables(String& code, const AST::Tree& ast, AST::Id owner)
	{
		auto variables = ast.MakeView<const CIdentifier, const CVariableDecl>();

		if (const CParent* parent = AST::GetCParent(ast, owner))
		{
			for (AST::Id entity : parent->children)
			{
				if (variables.Has(entity))
				{
					auto& identifier = variables.Get<const CIdentifier>(entity);
					AddVariable(code, "bool", identifier.name.ToString(), "false");
				}
			}
		}
	}

	void DeclareTypes(String& code, const AST::Tree& ast)
	{
		auto structs = ast.MakeView<const CIdentifier, const CStructDecl>();
		for (AST::Id entity : structs)
		{
			auto& identifier = structs.Get<const CIdentifier>(entity);
			DeclareStruct(code, identifier.name.ToString(), {}, [&ast, entity](String& innerCode) {
				AddTypeVariables(innerCode, ast, entity);
			});
		}

		auto classes = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : classes)
		{
			auto& identifier = classes.Get<const CIdentifier>(entity);
			DeclareStruct(code, identifier.name.ToString(), {}, [&ast, entity](String& innerCode) {
				AddTypeVariables(innerCode, ast, entity);
			});
		}
	}


	void DeclareFunctions(String& code, const AST::Tree& ast)
	{
		auto functions = ast.MakeView<const CIdentifier, const CFunctionDecl>();
		auto children  = ast.MakeView<const CChild>();
		auto classes   = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;

			const CChild* parent = AST::GetCChild(ast, entity);
			if (parent && ast.IsValid(parent->parent))
			{
				if (const CIdentifier* parentId = classes.TryGet<const CIdentifier>(parent->parent))
				{
					ownerName = parentId->name.ToString();
				}
			}

			const auto& identifier = functions.Get<const CIdentifier>(entity);
			DeclareFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void DefineFunctions(String& code, const AST::Tree& ast)
	{
		auto functions = ast.MakeView<const CIdentifier, const CFunctionDecl>();
		auto children  = ast.MakeView<const CChild>();
		auto classes   = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;

			const CChild* parent = AST::GetCChild(ast, entity);
			if (parent && ast.IsValid(parent->parent))
			{
				if (const CIdentifier* parentId = classes.TryGet<const CIdentifier>(parent->parent))
				{
					ownerName = parentId->name.ToString();
				}
			}

			auto& identifier = functions.Get<const CIdentifier>(entity);
			DefineFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void GenerateCode(Context& context, const Path& generatePath)
	{
		const auto& config = context.config;
		const auto& ast    = context.ast;

		const Path includePath = generatePath / "Include";
		const Path sourcePath  = generatePath / "Src";
		Files::CreateFolder(includePath, true);
		Files::CreateFolder(sourcePath, true);

		ZoneScopedC(0x459bd1);

		String code;

		Strings::FormatTo(code, "#pragma once\n");
		Spacing(code);

		// Includes
		AddInclude(code, "stdint.h");

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareTypes(code, ast);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, ast);

		Spacing(code);
		Comment(code, "Function Declarations");
		DeclareFunctions(code, ast);

		Spacing(code);
		Comment(code, "Function Definitions");
		DefineFunctions(code, ast);

		const Path headerFile = includePath / "code.h";
		if (!Files::SaveStringFile(headerFile, code))
		{
			context.AddError(Strings::Format(
			    "Couldn't save generated header at '{}'", Paths::ToString(headerFile)));
		}

		code = {};
		AddInclude(code, "code.h");
		Strings::FormatTo(code, "int main() {{ return 0; }}\n");

		Path sourceFile = sourcePath / "code.cpp";
		if (!Files::SaveStringFile(sourceFile, code))
		{
			context.AddError(Strings::Format(
			    "Couldn't save generated source at '{}'", Paths::ToString(sourceFile)));
		}
	}
}    // namespace Rift::Compiler::Cpp
