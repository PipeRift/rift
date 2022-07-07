// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend/CppGeneration.h"

#include "AST/Utils/Namespaces.h"
#include "CppBackend.h"
#include "CppBackend/Components/CCppCodeGenFragment.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CType.h>
#include <AST/Tree.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Context.h>
#include <Pipe/Core/String.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>


namespace rift::Compiler::Cpp
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

	void ForwardDeclareTypes(String& code, TAccessRef<CNamespace> access, AST::Id moduleId,
	    const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
		for (AST::Id entity : structs)
		{
			const auto& ns = access.Get<const CNamespace>(entity);
			ForwardDeclareStruct(code, ns.name.ToString());
		}

		for (AST::Id entity : classes)
		{
			const auto& ns = access.Get<const CNamespace>(entity);
			ForwardDeclareStruct(code, ns.name.ToString());
		}
	}

	void AddTypeVariables(
	    String& code, TAccessRef<CNamespace, CDeclVariable, CParent> access, AST::Id owner)
	{
		if (const auto* parent = access.TryGet<const CParent>(owner))
		{
			for (AST::Id entity : parent->children)
			{
				if (access.Has<const CNamespace, const CDeclVariable>(entity))
				{
					auto& ns = access.Get<const CNamespace>(entity);
					AddVariable(code, "bool", ns.name.ToString(), "false");
				}
			}
		}
	}

	void DeclareTypes(String& code, TAccessRef<CType, CNamespace, CDeclVariable, CParent> access,
	    AST::Id moduleId, const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
		for (AST::Id entity : structs)
		{
			const auto& ns = access.Get<const CNamespace>(entity);
			DeclareStruct(code, ns.name.ToString(), {}, [&access, entity](String& innerCode) {
				AddTypeVariables(innerCode, access, entity);
			});
		}

		for (AST::Id entity : classes)
		{
			const auto& ns = access.Get<const CNamespace>(entity);
			DeclareStruct(code, ns.name.ToString(), {}, [&access, entity](String& innerCode) {
				AddTypeVariables(innerCode, access, entity);
			});
		}
	}


	void DeclareFunctions(String& code,
	    TAccessRef<CType, CNamespace, CDeclClass, CChild, CModule> access, AST::Id moduleId,
	    const TArray<AST::Id>& functions)
	{
		for (AST::Id entity : functions)
		{
			AST::Namespace ns = AST::GetNamespace(access, entity);
			DeclareFunction(code, AST::GetNameChecked(access, entity).ToString(), ns.ToString());
		}
	}


	void DefineFunctions(String& code, TAccessRef<CNamespace, CType, CDeclClass, CChild> access,
	    AST::Id moduleId, const TArray<AST::Id>& functions)
	{
		for (AST::Id entity : functions)
		{
			StringView ownerName;
			AST::Id parentId = AST::Hierarchy::GetParent(access, entity);
			if (!IsNone(parentId))
			{
				if (const auto* ns = access.TryGet<const CNamespace>(parentId))
				{
					ownerName = ns->name.ToString();
				}
			}

			const auto& ns = access.Get<const CNamespace>(entity);
			DefineFunction(code, ns.name.ToString(), ownerName);
		}
	}

	void GenParameters(TAccessRef<CExprType, CNamespace, TWrite<CCppCodeGenFragment>> access) {}


	void GenerateModuleCode(Context& context, AST::Id moduleId, const p::Path& codePath)
	{
		ZoneScopedC(0x459bd1);

		auto& ast = context.ast;

		const Name name           = Modules::GetModuleName(ast, moduleId);
		const p::Path modulePath  = codePath / name.ToString();
		const p::Path includePath = modulePath / "Include";
		const p::Path sourcePath  = modulePath / "Src";
		files::CreateFolder(includePath, true);
		files::CreateFolder(sourcePath, true);


		String code;
		Strings::FormatTo(code, "#pragma once\n");
		Spacing(code);

		// Includes
		AddInclude(code, "stdint.h");

		TArray<AST::Id> classes, structs;
		AST::Hierarchy::GetChildren(ast, moduleId, classes);
		ecs::ExcludeIfNot<CType, CNamespace>(ast, classes);
		structs = classes;

		ecs::ExcludeIfNot<CDeclClass>(ast, classes);
		ecs::ExcludeIfNot<CDeclStruct>(ast, structs);

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareTypes(code, ast, moduleId, structs, classes);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, ast, moduleId, structs, classes);


		TArray<AST::Id> functions;
		// Module -> Types -> Functions = depth 2
		AST::Hierarchy::GetChildrenDeep(ast, moduleId, functions, 2);
		ecs::ExcludeIfNot<CNamespace, CDeclFunction>(ast, functions);

		Spacing(code);
		Comment(code, "Function Declarations");
		DeclareFunctions(code, ast, moduleId, functions);

		Spacing(code);
		Comment(code, "Function Definitions");
		DefineFunctions(code, ast, moduleId, functions);

		const p::Path headerFile = includePath / "code.h";
		if (!files::SaveStringFile(headerFile, code))
		{
			context.AddError(
			    Strings::Format("Couldn't save generated header at '{}'", p::ToString(headerFile)));
		}

		code = {};
		AddInclude(code, "code.h");
		Strings::FormatTo(code, "int main() {{ return 0; }}\n");
		Spacing(code);

		Path sourceFile = sourcePath / "code.cpp";
		if (!files::SaveStringFile(sourceFile, code))
		{
			context.AddError(
			    Strings::Format("Couldn't save generated source at '{}'", p::ToString(sourceFile)));
		}
	}

	void GenerateCode(Context& context, const p::Path& generatePath)
	{
		ZoneScopedC(0x459bd1);

		GenParameters(context.ast);

		for (AST::Id moduleId : ecs::ListAll<CModule>(context.ast))
		{
			GenerateModuleCode(context, moduleId, generatePath);
		}
	}
}    // namespace rift::Compiler::Cpp
