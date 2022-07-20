// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend/CMakeGen.h"

#include "CppBackend.h"

#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>


namespace rift::compiler::Cpp
{
	void SetProject(String& code, Compiler& compiler, Name name, StringView version)
	{
		Strings::FormatTo(code,
		    "cmake_minimum_required (VERSION 3.12)\n"
		    "project({ProjectName} VERSION {Version} LANGUAGES CXX)\n",
		    fmt::arg("ProjectName", name), fmt::arg("Version", version));
	}

	void AddSubdirectory(String& code, StringView path)
	{
		Strings::FormatTo(code, "add_subdirectory({})\n", path);
	}

	void AddLibrary(
	    String& code, Compiler& compiler, ModuleTarget type, Name name, StringView cppVersion)
	{
		StringView targetType;
		switch (type)
		{
			case ModuleTarget::Shared: targetType = "SHARED"; break;
			case ModuleTarget::Static: targetType = "STATIC"; break;
			// case ModuleTarget::Interface:
			// targetType = "INTERFACE";
			// break;
			default: compiler.AddError("Failed to add CMake library. Invalid ModuleTarget"); return;
		}

		Strings::FormatTo(code,
		    "add_library({ModuleName} {TargetType})\n"
		    "set_target_properties ({ModuleName} PROPERTIES CXX_STANDARD {CppVersion})\n"
		    "target_include_directories({ModuleName} PUBLIC Include)\n",
		    fmt::arg("ModuleName", name), fmt::arg("TargetType", targetType),
		    fmt::arg("CppVersion", cppVersion));


		Strings::FormatTo(code, "target_sources({ModuleName} PRIVATE Src/code.cpp)\n",
		    fmt::arg("ModuleName", name));
	}

	void AddExecutable(String& code, Compiler& compiler, Name name, StringView cppVersion)
	{
		Strings::FormatTo(code,
		    "add_executable({ModuleName})\n"
		    "set_target_properties ({ModuleName} PROPERTIES CXX_STANDARD {CppVersion})\n"
		    "target_include_directories({ModuleName} PUBLIC Include)\n",
		    fmt::arg("ModuleName", name), fmt::arg("CppVersion", cppVersion));

		Strings::FormatTo(code, "target_sources({ModuleName} PRIVATE Src/code.cpp)\n",
		    fmt::arg("ModuleName", name));
	}

	void GenerateCMakeModule(
	    Compiler& compiler, AST::Id moduleId, CModule& module, const p::Path& codePath, Name name)
	{
		String code;
		ModuleTarget target = module.target;
		if (target == ModuleTarget::Executable)
		{
			AddExecutable(code, compiler, name.ToString(), "20");
		}
		else
		{
			AddLibrary(code, compiler, target, name.ToString(), "20");
		}
		files::SaveStringFile(codePath / name.ToString() / "CMakelists.txt", code);
	}

	void GenerateCMake(Compiler& compiler, const p::Path& codePath)
	{
		String code;
		Name projectName = Modules::GetProjectName(compiler.ast);
		SetProject(code, compiler, projectName.ToString(), "0.1");

		TAccess<TWrite<CModule>> modules{compiler.ast};
		for (AST::Id moduleId : ecs::ListAll<CModule>(modules))
		{
			Name name = Modules::GetModuleName(compiler.ast, moduleId);
			GenerateCMakeModule(compiler, moduleId, modules.Get<CModule>(moduleId), codePath, name);

			AddSubdirectory(code, name.ToString());
		}

		files::SaveStringFile(codePath / "CMakelists.txt", code);
	}
}    // namespace rift::compiler::Cpp
