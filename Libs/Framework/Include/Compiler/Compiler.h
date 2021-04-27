// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/C/Backend_C.h"
#include "Compiler/CompilerContext.h"
#include "Project.h"

#include <CoreObject.h>


namespace Rift::Compiler
{
	enum class EBackend : u8
	{
		C,
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
			case EBackend::C:
				C::Build(project, config);
			case EBackend::LLVM:
				Log::Error("LLVM backend is not yet supported.");
				break;
			default:
				Log::Error("Unknown backend.");
		}
	}
}    // namespace Rift::Compiler
