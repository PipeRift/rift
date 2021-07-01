// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"
#include "Assets/ModuleAsset.h"
#include "Compiler/Cpp/CMakeGen.h"
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

	void GenerateCMake(Context& context, const Path& generatePath)
	{
		ModuleType type = ModuleType::Executable;

		Name projectName = Modules::GetProjectName(context.ast);

		String code;
		SetProject(code, context, projectName.ToString(), "0.1");
		if (type == ModuleType::Executable)
		{
			AddExecutable(code, context, projectName.ToString(), "14");
		}
		else
		{
			AddLibrary(code, context, type, projectName.ToString(), "14");
		}

		Files::SaveStringFile(generatePath / "CMakelists.txt", code);
	}
}    // namespace Rift::Compiler::Cpp
