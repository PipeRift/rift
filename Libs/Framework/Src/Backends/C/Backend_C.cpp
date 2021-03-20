// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/C/Backend_C.h"
#include "Files/FileSystem.h"


namespace Rift::Backends::C
{
	void AddInclude(String& code, StringView name)
	{
		CString::FormatTo(code, "#include <{}>\n", name);
	}

	void ForwardDeclareStruct(String& code, StringView name)
	{
		CString::FormatTo(code, "struct {};\n", name);
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
			CString::FormatTo(code, "struct {}\n{{\n{} super;\n{}}};\n\n", name, super, innerCode);
		}
		else
		{
			CString::FormatTo(code, "struct {}\n{{\n{}}};\n\n", name, innerCode);
		}
	}

	void AddVariable(String& code, StringView type, StringView name)
	{
		CString::FormatTo(code, "{} {};\n", type, name);
	}


	void GenerateCode(CCompilerContext& context)
	{
		ZoneScopedC(0x459bd1);
		auto& config = context.config;
		String code;

		// Includes
		AddInclude(code, "stdint.h");

		ForwardDeclareStruct(code, "MyStruct");
		ForwardDeclareStruct(code, "ChildrenStruct");

		AddStruct(code, "MyStruct", {}, [](String& innerCode) {
			AddVariable(innerCode, "char", "alive");
		});

		AddStruct(code, "ChildrenStruct", "MyStruct", [](String& innerCode) {
			AddVariable(innerCode, "char", "done");
		});

		Path codePath = config.intermediatesPath / "code.h";
		if (!FileSystem::SaveStringFile(codePath, code))
		{
			context.AddError(CString::Format(
			    "Couldn't save generated code at '{}'", FileSystem::ToString(codePath)));
		}
	}
	void Build() {}

	void Compile(Ptr<Project> project, const CompilerConfig& config)
	{
		ZoneScopedC(0x459bd1);

		if (!project)
		{
			return;
		}

		Log::Info("Building project '{}'", project->GetName());

		CCompilerContext context{};
		context.project = project;
		context.config  = config;
		context.config.Init(project);

		FileSystem::Delete(config.intermediatesPath, true, false);
		FileSystem::Delete(config.binariesPath, true, false);
		FileSystem::CreateFolder(config.intermediatesPath, true);
		FileSystem::CreateFolder(config.binariesPath, true);

		project->LoadAllAssets();

		GenerateCode(context);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate C code.");
			return;
		}
		Log::Info("Generated C code");

		Build();
		if (context.HasErrors())
		{
			context.AddError("Failed to compile C code.");
		}
	}
}    // namespace Rift::Backends::C
