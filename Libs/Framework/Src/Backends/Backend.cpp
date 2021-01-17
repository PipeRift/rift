// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/Backend.h"

namespace Rift::Backends
{
	void Backend::SetProject(Ptr<Project> inProject)
	{
		project = inProject;
		if (project.IsValid())
		{
			buildPath = project->GetPath() / "Build";
			intermediatesPath = buildPath / "Intermediates";
			binariesPath = buildPath / "Binaries";
		}
	}

	void Backend::Compile()
	{
		// Reset any previous errors
		errors.Empty();

		OnCompile();
		OnCleanup();
	}
}	 // namespace Rift::Backends