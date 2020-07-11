// Copyright 2015-2020 Piperift - All rights reserved

#include "Backends/C/Backend_C.h"


namespace VCLang::Backends::C
{
	void Backend_C::Generate(Ptr<Project> inProject)
	{
		project = inProject;
	}

	void Backend_C::Build() {}
}	 // namespace VCLang::Backends::C
