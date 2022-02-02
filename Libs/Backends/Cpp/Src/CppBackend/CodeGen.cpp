// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend/CodeGen.h"

#include "CppBackend.h"
#include "CppBackend/Components/CCppCodeGenFragment.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CClassDecl.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CParameterDecl.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CType.h>
#include <AST/Components/CVariableDecl.h>
#include <AST/Tree.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Context.h>
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

	void ForwardDeclareTypes(String& code, const AST::Tree& ast, AST::Id moduleId,
	    const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
		auto types = ast.Filter<const CType>();
		for (AST::Id entity : structs)
		{
			const auto& type = types.Get<const CType>(entity);
			ForwardDeclareStruct(code, type.name.ToString());
		}

		for (AST::Id entity : classes)
		{
			const auto& type = types.Get<const CType>(entity);
			ForwardDeclareStruct(code, type.name.ToString());
		}
	}

	void AddTypeVariables(String& code, const AST::Tree& ast, AST::Id owner)
	{
		auto variables = ast.Filter<const CIdentifier, const CVariableDecl>();

		if (const CParent* parent = AST::Hierarchy::GetCParent(ast, owner))
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

	void DeclareTypes(String& code, const AST::Tree& ast, AST::Id moduleId,
	    const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
		auto types = ast.Filter<const CType>();
		for (AST::Id entity : structs)
		{
			const auto& type = types.Get<const CType>(entity);
			DeclareStruct(code, type.name.ToString(), {}, [&ast, entity](String& innerCode) {
				AddTypeVariables(innerCode, ast, entity);
			});
		}

		for (AST::Id entity : classes)
		{
			const auto& type = types.Get<const CType>(entity);
			DeclareStruct(code, type.name.ToString(), {}, [&ast, entity](String& innerCode) {
				AddTypeVariables(innerCode, ast, entity);
			});
		}
	}


	void DeclareFunctions(
	    String& code, const AST::Tree& ast, AST::Id moduleId, const TArray<AST::Id>& functions)
	{
		auto identifiers = ast.Filter<const CIdentifier>();
		auto classesView = ast.Filter<const CType, const CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;
			AST::Id parentId = AST::Hierarchy::GetParent(ast, entity);
			if (!IsNone(parentId) && classesView.Has(parentId))
			{
				if (auto* type = classesView.TryGet<const CType>(parentId))
				{
					ownerName = type->name.ToString();
				}
			}

			const auto& identifier = identifiers.Get<const CIdentifier>(entity);
			DeclareFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void DefineFunctions(
	    String& code, const AST::Tree& ast, AST::Id moduleId, const TArray<AST::Id>& functions)
	{
		auto identifiers = ast.Filter<const CIdentifier>();
		auto classesView = ast.Filter<const CType, const CClassDecl>();
		for (AST::Id entity : functions)
		{
			StringView ownerName;
			AST::Id parentId = AST::Hierarchy::GetParent(ast, entity);
			if (!IsNone(parentId) && classesView.Has(parentId))
			{
				if (auto* type = classesView.TryGet<const CType>(parentId))
				{
					ownerName = type->name.ToString();
				}
			}

			auto& identifier = identifiers.Get<const CIdentifier>(entity);
			DefineFunction(code, identifier.name.ToString(), ownerName);
		}
	}

	void GenParameters(AST::Tree& ast)
	{
		auto parameters = ast.Filter<const CParameterDecl, const CIdentifier>();
		for (AST::Id entity : parameters)
		{
			const auto& param = parameters.Get<const CParameterDecl>(entity);
			const auto& name  = parameters.Get<const CIdentifier>(entity);

			auto& fragment = ast.Add<CCppCodeGenFragment>(entity);
			fragment.code.clear();
			Strings::FormatTo(fragment.code, "{} {}", param.type, name.name);
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

		TArray<AST::Id> classes, structs;
		AST::Hierarchy::GetChildren(ast, moduleId, classes);
		structs = classes;

		auto classesView = ast.Filter<const CClassDecl, const CType>();
		classesView.FilterIds(classes);

		auto structsView = ast.Filter<const CStructDecl, const CType>();
		structsView.FilterIds(structs);

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareTypes(code, ast, moduleId, structs, classes);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, ast, moduleId, structs, classes);


		TArray<AST::Id> functions;
		// Module->Types->Functions = depth 2
		AST::Hierarchy::GetChildrenDeep(ast, moduleId, functions, 2);

		auto functionsView = ast.Filter<const CIdentifier, const CFunctionDecl>();
		functions.RemoveIfSwap([&functionsView](AST::Id entity) {
			return !functionsView.Has(entity);
		});

		Spacing(code);
		Comment(code, "Function Declarations");
		DeclareFunctions(code, ast, moduleId, functions);

		Spacing(code);
		Comment(code, "Function Definitions");
		DefineFunctions(code, ast, moduleId, functions);

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

		GenParameters(context.ast);

		auto modules = context.ast.Filter<CModule>();
		for (AST::Id moduleId : modules)
		{
			GenerateModuleCode(context, moduleId, generatePath);
		}
	}
}    // namespace Rift::Compiler::Cpp