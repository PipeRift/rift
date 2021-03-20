// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/Backend.h"

namespace Rift::Backends
{
	void CompilerConfig::Init(Ptr<Project> rootProject)
	{
		if (rootProject.IsValid())
		{
			buildPath         = rootProject->GetPath() / "Build";
			intermediatesPath = buildPath / "Intermediates";
			binariesPath      = buildPath / "Binaries";
		}
	}

	void CompilerContext::AddError(StringView str)
	{
		Log::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}
}    // namespace Rift::Backends
