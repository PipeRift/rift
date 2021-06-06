// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerContext.h"
#include "Compiler/Cpp/Backend_Cpp.h"
#include "Project.h"

#include <CoreObject.h>


namespace Rift::Compiler
{
	enum class EBackend : u8
	{
		Cpp,
		LLVM
	};


	void Build(TPtr<Project> project, const Config& config, EBackend backend)
	{
		if (!project)
		{
			return;
		}

		switch (backend)
		{
			case EBackend::Cpp:
				Cpp::Build(project, config);
			case EBackend::LLVM:
				Log::Error("LLVM backend is not yet supported.");
				break;
			default:
				Log::Error("Unknown backend.");
		}
	}
}    // namespace Rift::Compiler
