// Copyright 2015-2020 Piperift - All rights reserved

#include "Assets/ProjectAsset.h"
#include "Compiler/Cpp/CMakeGenerator.h"
#include "Compiler/Cpp/CppBackend.h"

#include <Files/Files.h>


namespace Rift::Compiler::Cpp
{
	void SetProject(String& code, Context& context, Name name, StringView version)
	{
		Strings::FormatTo(code,
		    "cmake_minimum_required (VERSION 3.12)\n"
		    "project({ProjectName} VERSION {Version} LANGUAGES CXX)\n",
		    fmt::arg("ProjectName", name), fmt::arg("Version", version));
	}

	void AddLibrary(
	    String& code, Context& context, ModuleType type, Name name, StringView cppVersion)
	{
		StringView targetType;
		switch (type)
		{
			case ModuleType::Shared:
				targetType = "SHARED";
				break;
			case ModuleType::Static:
				targetType = "STATIC";
				break;
			case ModuleType::Interface:
				targetType = "INTERFACE";
				break;
			default:
				context.AddError("Failed to add CMake library. Invalid ModuleType");
				return;
		}

		Strings::FormatTo(code,
		    "add_library({ModuleName} {TargetType})\n"
		    "set_target_properties ({ModuleName} PROPERTIES CXX_STANDARD {CppVersion})\n"
		    "target_include_directories({ModuleName} PUBLIC Include)\n",
		    fmt::arg("ModuleName", name), fmt::arg("TargetType", targetType),
		    fmt::arg("CppVersion", cppVersion));


		if (type != ModuleType::Interface)
		{
			Strings::FormatTo(code, "target_sources({ModuleName} PRIVATE Src/code.cpp)\n",
			    fmt::arg("ModuleName", name));
		}
	}

	void AddExecutable(String& code, Context& context, Name name, StringView cppVersion)
	{
		Strings::FormatTo(code,
		    "add_executable({ModuleName})\n"
		    "set_target_properties ({ModuleName} PROPERTIES CXX_STANDARD {CppVersion})\n"
		    "target_include_directories({ModuleName} PUBLIC Include)\n",
		    fmt::arg("ModuleName", name), fmt::arg("CppVersion", cppVersion));

		Strings::FormatTo(code, "target_sources({ModuleName} PRIVATE Src/code.cpp)\n",
		    fmt::arg("ModuleName", name));
	}

	void GenerateCMake(Context& context)
	{
		ModuleType type = ModuleType::Executable;

		String code;
		SetProject(code, context, context.project->GetName().ToString(), "0.1");
		if (type == ModuleType::Executable)
		{
			AddExecutable(code, context, context.project->GetName().ToString(), "14");
		}
		else
		{
			AddLibrary(code, context, type, context.project->GetName().ToString(), "14");
		}

		Files::SaveStringFile(context.config.intermediatesPath / "CMakelists.txt", code);
	}
}    // namespace Rift::Compiler::Cpp
