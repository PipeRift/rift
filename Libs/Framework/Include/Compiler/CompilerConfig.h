// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Module.h"

#include <CoreObject.h>
#include <Reflection/Reflection.h>


namespace Rift::Compiler
{
	struct Config : public Struct
	{
		STRUCT(Config, Struct)

		String buildMode{"Release"};

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(TPtr<Module> rootProject);
	};
}    // namespace Rift::Compiler
