// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/C/Backend_C.h"

#include "Files/FileSystem.h"


namespace Rift::Backends::C
{
	void Backend_C::OnCompile()
	{
		ZoneScopedC(0x459bd1);
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
		ZoneScopedC(0x459bd1);
		code = "";

		// Includes
		AddInclude("stdint.h");

		// Generate class
		{
			ForwardDeclareClass("MyClass");
			ForwardDeclareClass("MyClass33");
			ForwardDeclareStruct("MyStruct");

			BeginClass("MyClass");
			EndClass();
			BeginClass("MyClass33");
			EndClass();
			BeginStruct("MyStruct");
			EndStruct();
		}

		FileSystem::SaveStringFile(intermediatesPath / "code.h", code);
	}

	void Backend_C::Build()
	{
		ZoneScopedC(0x459bd1);
	}

	void Backend_C::OnCleanup()
	{
		ZoneScopedC(0x459bd1);
		Super::OnCleanup();
	}


	void Backend_C::AddInclude(StringView name)
	{
		CString::FormatTo(code, "#include <{}>\n", name);
	}

	void Backend_C::ForwardDeclareClass(StringView name)
	{
		CString::FormatTo(code, "class {};\n\n", name);
	}

	void Backend_C::BeginClass(StringView name)
	{
		CString::FormatTo(code, "class {}\n{{\n", name);
	}
	void Backend_C::EndClass()
	{
		CString::FormatTo(code, "}};\n\n");
	}

	void Backend_C::ForwardDeclareStruct(StringView name)
	{
		CString::FormatTo(code, "struct {};\n\n", name);
	}
	void Backend_C::BeginStruct(StringView name)
	{
		CString::FormatTo(code, "struct {}\n{{\n", name);
	}

	void Backend_C::EndStruct()
	{
		CString::FormatTo(code, "}};\n\n");
	}
}    // namespace Rift::Backends::C
