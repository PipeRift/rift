// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Compiler/Compiler.h"

#include <Pipe/Reflect/Class.h>


namespace rift
{
	class Backend : public p::Class
	{
		P_CLASS(Backend, p::Class)

	public:
		virtual p::Tag GetName()
		{
			return p::Tag::None();
		}

		virtual void Build(Compiler& compiler)
		{
			CheckMsg(false, "Backend '{}' tried to run but Build() is not implemented.",
			    GetName().AsString());
		}
	};
}    // namespace rift
