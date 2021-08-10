// Copyright 2015-2020 Piperift - All rights reserved

#include "Compiler/Cpp/CodeGen.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Linkage.h"
#include "AST/Tree.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/CompilerContext.h"
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

	void ForwardDeclareTypes(String& code, const AST::Tree& ast, AST::Id moduleId)
	{
		const auto* children = AST::GetLinked(ast, moduleId);
		if (!children)
		{
			return;
		}

		auto structs = ast.MakeView<const CIdentifier, const CStructDecl>();
		for (AST::Id entity : *children)
		{
			if (structs.Has(entity))
			{
				const auto& identifier = structs.Get<const CIdentifier>(entity);
				ForwardDeclareStruct(code, identifier.name.ToString());
			}
		}

		auto classes = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : *children)
		{
			if (classes.Has(entity))
			{
				const auto& identifier = classes.Get<const CIdentifier>(entity);
				ForwardDeclareStruct(code, identifier.name.ToString());
			}
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

	void DeclareTypes(String& code, const AST::Tree& ast, AST::Id moduleId)
	{
		const auto* children = AST::GetLinked(ast, moduleId);
		if (!children)
		{
			return;
		}

		auto structs = ast.MakeView<const CIdentifier, const CStructDecl>();
		for (AST::Id entity : *children)
		{
			if (structs.Has(entity))
			{
				auto& identifier = structs.Get<const CIdentifier>(entity);
				DeclareStruct(
				    code, identifier.name.ToString(), {}, [&ast, entity](String& innerCode) {
					    AddTypeVariables(innerCode, ast, entity);
				    });
			}
		}

		auto classes = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : *children)
		{
			if (classes.Has(entity))
			{
				auto& identifier = classes.Get<const CIdentifier>(entity);
				DeclareStruct(
				    code, identifier.name.ToString(), {}, [&ast, entity](String& innerCode) {
					    AddTypeVariables(innerCode, ast, entity);
				    });
			}
		}
	}


	void DeclareFunctions(String& code, const AST::Tree& ast, AST::Id moduleId)
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


	void DefineFunctions(String& code, const AST::Tree& ast, AST::Id moduleId)
	{
		auto functions = ast.MakeView<const CIdentifier, const CFunctionDecl>();
		auto children  = ast.MakeView<const CChild>();
		auto classes   = ast.MakeView<const CIdentifier, const CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;

			const CChild* child = AST::GetCChild(ast, entity);
			if (child && ast.IsValid(child->parent))
			{
				if (const CIdentifier* parentId = classes.TryGet<const CIdentifier>(child->parent))
				{
					ownerName = parentId->name.ToString();
				}
			}

			auto& identifier = functions.Get<const CIdentifier>(entity);
			DefineFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void GenerateModuleCode(Context& context, AST::Id moduleId, const Path& codePath)
	{
		ZoneScopedC(0x459bd1);

		const auto& config = context.config;
		const auto& ast    = context.ast;

		const Name name        = Modules::GetModuleName(ast, moduleId);
		const Path modulePath  = codePath / name.ToString();
		const Path includePath = modulePath / "Include";
		const Path sourcePath  = modulePath / "Src";
		Files::CreateFolder(includePath, true);
		Files::CreateFolder(sourcePath, true);


		String code;
		Strings::FormatTo(code, "#pragma once\n");
		Spacing(code);

		// Includes
		AddInclude(code, "stdint.h");

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareTypes(code, ast, moduleId);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, ast, moduleId);

		Spacing(code);
		Comment(code, "Function Declarations");
		// DeclareFunctions(code, ast, moduleId);

		Spacing(code);
		Comment(code, "Function Definitions");
		// DefineFunctions(code, ast, moduleId);

		const Path headerFile = includePath / "code.h";
		if (!Files::SaveStringFile(headerFile, code))
		{
			context.AddError(Strings::Format(
			    "Couldn't save generated header at '{}'", Paths::ToString(headerFile)));
		}

		code = {};
		AddInclude(code, "code.h");
		Strings::FormatTo(code, "int main() {{ return 0; }}\n");
		Spacing(code);

		Path sourceFile = sourcePath / "code.cpp";
		if (!Files::SaveStringFile(sourceFile, code))
		{
			context.AddError(Strings::Format(
			    "Couldn't save generated source at '{}'", Paths::ToString(sourceFile)));
		}
	}

	void GenerateCode(Context& context, const Path& generatePath)
	{
		ZoneScopedC(0x459bd1);

		auto modules = context.ast.MakeView<CModule>();
		for (AST::Id moduleId : modules)
		{
			GenerateModuleCode(context, moduleId, generatePath);
		}
	}
}    // namespace Rift::Compiler::Cpp
