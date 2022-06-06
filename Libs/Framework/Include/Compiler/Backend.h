// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/Context.h"

#include <PRefl/Class.h>


namespace rift::Compiler
{
	class Backend : public Class
	{
		CLASS(Backend, Class)

	public:
		virtual Name GetName()
		{
			return Name::None();
		}

		virtual void Build(Context& context)
		{
			CheckMsg(false, "Backend '{}' tried to run but Build() is not implemented.",
			    GetName().ToString());
		}
	};
}    // namespace rift::Compiler
