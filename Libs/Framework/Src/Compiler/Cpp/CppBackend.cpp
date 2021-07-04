// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Components/CClassDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CModule.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Cpp/CMakeGen.h"
#include "Compiler/Cpp/CodeGen.h"
#include "Compiler/Cpp/CppBackend.h"

#include <Files/Files.h>


namespace Rift::Compiler::Cpp
{
	void BuildCode(Context& context, const Path& codePath, const Path& cmakePath)
	{
		ZoneScopedC(0x459bd1);
		Files::CreateFolder(cmakePath, true);


		Log::Info("Generating");
		const String generate = Strings::Format(
		    "cmake -S {} -B {}", Paths::ToString(codePath), Paths::ToString(cmakePath));

		if (std::system(generate.c_str()) > 0)
		{
			context.AddError("Failed to generate cmake.");
			return;
		}

		Log::Info("Building");
		const String build = Strings::Format(
		    "cmake --build {} --config {}", Paths::ToString(cmakePath), context.config.buildMode);
		if (std::system(build.c_str()) > 0)
		{
			context.AddError("Failed to generate cmake.");
			return;
		}
	}

	void BuildModule(Context& context, AST::Id moduleId)
	{
		CModule* module = context.ast.TryGet<CModule>(moduleId);
		if (!module)
		{
			Log::Info("Cant find module");
			return;
		}

		Log::Info("Building module '{}'", Modules::GetModuleName(*module));
		Log::Info("Loading module files");
		// project->LoadAllAssets();
	}

	void Build(Context& context, const Config& tmpConfig)
	{
		ZoneScopedC(0x459bd1);

		Log::Info("Building project '{}'", Modules::GetProjectName(context.ast));

		// context.project = project;
		context.config  = tmpConfig;
		// context.config.Init(project);

		const Path& codePath = context.config.intermediatesPath / "Code";
		Files::Delete(codePath, true, false);
		Files::Delete(context.config.binariesPath, true, false);
		Files::CreateFolder(codePath, true);
		Files::CreateFolder(context.config.binariesPath, true);

		Log::Info("Loading files");
		// project->LoadAllAssets();

		Log::Info("Generating Code: C++");
		GenerateCode(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate C++ code.");
			return;
		}


		Log::Info("Generating code: CMake");
		GenerateCMake(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate C++ code.");
			return;
		}


		Log::Info("Building C++");
		const Path cmakePath = context.config.intermediatesPath / "CMake";
		BuildCode(context, codePath, cmakePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to compile C++ code.");
			return;
		}


		// Copy code & binaries
		if (!Files::Copy(codePath, context.config.binariesPath))
		{
			context.AddError("Failed to copy code");
			return;
		}
		if (!Files::Copy(cmakePath / context.config.buildMode, context.config.binariesPath / "Bin"))
		{
			context.AddError("Failed to copy binaries");
			return;
		}

		Log::Info("Build complete");
	}
}    // namespace Rift::Compiler::Cpp
