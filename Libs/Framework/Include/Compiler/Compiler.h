// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"
#include "Module.h"

#include <CoreObject.h>


namespace Rift::Compiler
{
	enum class EBackend : u8
	{
		Cpp,
		LLVM
	};


	void Build(TPtr<Module> project, const Config& config, EBackend backend);
}    // namespace Rift::Compiler
