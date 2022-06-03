// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend/CppGeneration.h"

#include "CppBackend.h"
#include "CppBackend/Components/CCppCodeGenFragment.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CType.h>
#include <AST/Tree.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Context.h>
#include <ECS/Filtering.h>
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

	void ForwardDeclareTypes(String& code, TAccessRef<CType> types, AST::Id moduleId,
	    const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
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

	void AddTypeVariables(
	    String& code, TAccessRef<CIdentifier, CDeclVariable, CParent> access, AST::Id owner)
	{
		if (const auto* parent = access.TryGet<const CParent>(owner))
		{
			for (AST::Id entity : parent->children)
			{
				if (access.Has<const CIdentifier, const CDeclVariable>(entity))
				{
					auto& identifier = access.Get<const CIdentifier>(entity);
					AddVariable(code, "bool", identifier.name.ToString(), "false");
				}
			}
		}
	}

	void DeclareTypes(String& code, TAccessRef<CType, CIdentifier, CDeclVariable, CParent> access,
	    AST::Id moduleId, const TArray<AST::Id>& structs, const TArray<AST::Id>& classes)
	{
		for (AST::Id entity : structs)
		{
			const auto& type = access.Get<const CType>(entity);
			DeclareStruct(code, type.name.ToString(), {}, [&access, entity](String& innerCode) {
				AddTypeVariables(innerCode, access, entity);
			});
		}

		for (AST::Id entity : classes)
		{
			const auto& type = access.Get<const CType>(entity);
			DeclareStruct(code, type.name.ToString(), {}, [&access, entity](String& innerCode) {
				AddTypeVariables(innerCode, access, entity);
			});
		}
	}


	void DeclareFunctions(String& code, TAccessRef<CIdentifier, CType, CDeclClass, CChild> access,
	    AST::Id moduleId, const TArray<AST::Id>& functions)
	{
		for (AST::Id entity : functions)
		{
			StringView ownerName;
			AST::Id parentId = AST::Hierarchy::GetParent(access, entity);
			if (!IsNone(parentId))
			{
				if (auto* type = access.TryGet<const CType>(parentId))
				{
					ownerName = type->name.ToString();
				}
			}

			const auto& identifier = access.Get<const CIdentifier>(entity);
			DeclareFunction(code, identifier.name.ToString(), ownerName);
		}
	}


	void DefineFunctions(String& code, TAccessRef<CIdentifier, CType, CDeclClass, CChild> access,
	    AST::Id moduleId, const TArray<AST::Id>& functions)
	{
		for (AST::Id entity : functions)
		{
			StringView ownerName;
			AST::Id parentId = AST::Hierarchy::GetParent(access, entity);
			if (!IsNone(parentId))
			{
				if (const auto* type = access.TryGet<const CType>(parentId))
				{
					ownerName = type->name.ToString();
				}
			}

			const auto& identifier = access.Get<const CIdentifier>(entity);
			DefineFunction(code, identifier.name.ToString(), ownerName);
		}
	}

	void GenParameters(TAccessRef<CExprType, CIdentifier, TWrite<CCppCodeGenFragment>> access) {}


	void GenerateModuleCode(Context& context, AST::Id moduleId, const Path& codePath)
	{
		ZoneScopedC(0x459bd1);

		auto& ast = context.ast;

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
		ECS::ExcludeIfNot<CType>(ast, classes);
		structs = classes;

		ECS::ExcludeIfNot<CDeclClass>(ast, classes);
		ECS::ExcludeIfNot<CDeclStruct>(ast, structs);

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareTypes(code, ast, moduleId, structs, classes);

		Spacing(code);
		Comment(code, "Declarations");
		DeclareTypes(code, ast, moduleId, structs, classes);


		TArray<AST::Id> functions;
		// Module -> Types -> Functions = depth 2
		AST::Hierarchy::GetChildrenDeep(ast, moduleId, functions, 2);
		ECS::ExcludeIfNot<CIdentifier, CDeclFunction>(ast, functions);

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

		for (AST::Id moduleId : ECS::ListAll<CModule>(context.ast))
		{
			GenerateModuleCode(context, moduleId, generatePath);
		}
	}
}    // namespace Rift::Compiler::Cpp
