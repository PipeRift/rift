// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Project.h"

#include <CoreObject.h>
#include <Reflection/Reflection.h>


namespace Rift::Compiler
{
	struct Config : public Struct
	{
		STRUCT(Config, Struct)

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(TPtr<Project> rootProject);
	};
}    // namespace Rift::Compiler
