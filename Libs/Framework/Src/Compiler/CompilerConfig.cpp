// Copyright 2015-2020 Piperift - All rights reserved

#include "Compiler/CompilerContext.h"


namespace Rift::Compiler
{
	void Config::Init(TPtr<Module> rootProject)
	{
		if (rootProject.IsValid())
		{
			buildPath         = rootProject->GetPath() / "Build";
			intermediatesPath = buildPath / "Intermediates";
			binariesPath      = buildPath / buildMode;
		}
	}
}    // namespace Rift::Compiler
