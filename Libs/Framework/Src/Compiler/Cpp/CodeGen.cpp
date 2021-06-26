// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Linkage.h"
#include "Compiler/Cpp/CodeGen.h"
#include "Compiler/Cpp/CppBackend.h"

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

	void ForwardDeclareTypes(String& code, Context& context)
	{
		auto& ast = context.ast;

		auto structs = ast.MakeView<CIdentifier, CStructDecl>();
		for (AST::Id entity : structs)
		{
			auto& identifier = structs.Get<CIdentifier>(entity);
			ForwardDeclareStruct(code, identifier.name.ToString());
		}

		auto classes = ast.MakeView<CIdentifier, CClassDecl>();
		for (AST::Id entity : classes)
		{
			auto& identifier = classes.Get<CIdentifier>(entity);
			ForwardDeclareStruct(code, identifier.name.ToString());
		}
	}

	void AddTypeVariables(String& code, Context& context, AST::Id owner)
	{
		auto& ast = context.ast;

		auto variables = ast.MakeView<CIdentifier, CVariableDecl>();

		if (const CParent* parent = AST::GetCParent(ast, owner))
		{
			for (AST::Id entity : parent->children)
			{
				if (variables.Has(entity))
				{
					auto& identifier = variables.Get<CIdentifier>(entity);
					AddVariable(code, "bool", identifier.name.ToString(), "false");
				}
			}
		}
	}

	void DeclareTypes(String& code, Context& context)
	{
		auto& ast = context.ast;

		auto structs = ast.MakeView<CIdentifier, CStructDecl>();
		for (AST::Id entity : structs)
		{
			auto& identifier = structs.Get<CIdentifier>(entity);
			DeclareStruct(
			    code, identifier.name.ToString(), {}, [&context, entity](String& innerCode) {
				    AddTypeVariables(innerCode, context, entity);
			    });
		}

		auto classes = ast.MakeView<CIdentifier, CClassDecl>();
		for (AST::Id entity : classes)
		{
			auto& identifier = classes.Get<CIdentifier>(entity);
			DeclareStruct(
			    code, identifier.name.ToString(), {}, [&context, entity](String& innerCode) {
				    AddTypeVariables(innerCode, context, entity);
			    });
		}
	}


	void DeclareFunctions(String& code, Context& context)
	{
		auto& ast = context.ast;

		auto functions = ast.MakeView<CIdentifier, CFunctionDecl>();
		auto children  = ast.MakeView<CChild>();
		auto classes   = ast.MakeView<CIdentifier, CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;

			const CChild* parent = AST::GetCChild(ast, entity);
			if (parent && ast.IsValid(parent->parent))
			{
				if (CIdentifier* parentIdent = classes.TryGet<CIdentifier>(parent->parent))
				{
					ownerName = parentIdent->name.ToString();
				}
			}

			auto& identifier = functions.Get<CIdentifier>(entity);
			DeclareFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void DefineFunctions(String& code, Context& context)
	{
		auto& ast = context.ast;

		auto functions = ast.MakeView<CIdentifier, CFunctionDecl>();
		auto children  = ast.MakeView<CChild>();
		auto classes   = ast.MakeView<CIdentifier, CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;

			const CChild* parent = AST::GetCChild(ast, entity);
			if (parent && ast.IsValid(parent->parent))
			{
				if (CIdentifier* parentIdent = classes.TryGet<CIdentifier>(parent->parent))
				{
					ownerName = parentIdent->name.ToString();
				}
			}

			auto& identifier = functions.Get<CIdentifier>(entity);
			DefineFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void GenerateCode(Context& context, const Path& generatePath)
	{
		auto& config = context.config;

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
		ForwardDeclareTypes(code, context);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, context);

		Spacing(code);
		Comment(code, "Function Declarations");
		DeclareFunctions(code, context);

		Spacing(code);
		Comment(code, "Function Definitions");
		DefineFunctions(code, context);

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
