// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Compiler.h"

#include <Pipe/Reflect/Class.h>
#include <Rift.h>


namespace rift::compiler
{
	class Backend : public Class
	{
		CLASS(Backend, Class)

	public:
		virtual Name GetName()
		{
			return Name::None();
		}

		virtual void Build(Compiler& compiler)
		{
			CheckMsg(false, "Backend '{}' tried to run but Build() is not implemented.",
			    GetName().ToString());
		}
	};
}    // namespace rift::compiler
