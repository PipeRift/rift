// Copyright 2015-2020 Piperift - All rights reserved

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
	void AddStruct(String& code, StringView name, StringView super = {},
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
			Strings::FormatTo(code, "void {} ();\n", name);
		}
		else
		{
			Strings::FormatTo(code, "void {} ({}& self);\n", name, owner);
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
			    "inline void {} ()\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    name, innerCode);
		}
		else
		{
			Strings::FormatTo(code,
			    "inline void {} ({}& self)\n"
			    "{{\n"
			    "{}"
			    "}};\n",
			    name, owner, innerCode);
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

		// Includes
		AddInclude(code, "stdint.h");

		Spacing(code);
		Comment(code, "Forward declarations");
		ForwardDeclareStruct(code, "MyStruct");
		ForwardDeclareStruct(code, "ChildrenStruct");
		ForwardDeclareStruct(code, "Another");


		Spacing(code);
		Comment(code, "Declarations");
		AddStruct(code, "MyStruct", {}, [](String& innerCode) {
			AddVariable(innerCode, "char", "alive", {});
		});

		AddStruct(code, "ChildrenStruct", "MyStruct", [](String& innerCode) {
			AddVariable(innerCode, "char", "done", {});
		});

		AddStruct(code, "Another", {}, [](String& innerCode) {
			AddVariable(innerCode, "ChildrenStruct", "data", {});
		});


		Spacing(code);
		Comment(code, "Function Declarations");
		DeclareFunction(code, "Move", "MyStruct");
		DeclareFunction(code, "MoveAll", {});


		Spacing(code);
		Comment(code, "Function Definitions");
		DefineFunction(code, "Move", "MyStruct");
		DefineFunction(code, "MoveAll", {});


		Path headerFile = includePath / "code.h";
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
