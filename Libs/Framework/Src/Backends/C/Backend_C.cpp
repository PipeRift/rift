// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/C/Backend_C.h"

#include "Files/FileSystem.h"


namespace VCLang::Backends::C
{
	void Backend_C::OnCompile()
	{
		ScopedZone(0x459bd1);
		if (!project)
		{
			return;
		}

		FileSystem::CreateFolder(intermediatesPath, true);
		FileSystem::CreateFolder(binariesPath, true);

		project->LoadAllAssets();

		Generate();
		if (HasErrors())
		{
			AddError("Failed to generate C code.");
			return;
		}

		Build();
		if (HasErrors())
		{
			AddError("Failed to compile C code.");
		}
	}

	void Backend_C::Generate()
	{
		ScopedZone(0x459bd1);

		String code;

		// Includes
		code += "#include <stdint.h>\n";

		// Generate class
		{
			StringView className = "MyClass";
			CString::FormatTo(code, "struct {} {{}};", className);
		}

		FileSystem::SaveStringFile(intermediatesPath / "code.h", code);
	}

	void Backend_C::Build()
	{
		ScopedZone(0x459bd1);
	}

	void Backend_C::OnCleanup()
	{
		ScopedZone(0x459bd1);
		Super::OnCleanup();
	}
}	 // namespace VCLang::Backends::C
