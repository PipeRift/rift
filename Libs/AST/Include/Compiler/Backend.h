// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Compiler/Compiler.h"

#include <PipeReflect.h>


namespace rift
{
	class Backend : public p::Object
	{
	public:
		using Super = p::Object;
		P_CLASS(Backend)

		virtual p::Tag GetName()
		{
			return p::Tag::None();
		}

		virtual void Build(Compiler& compiler)
		{
			P_CheckMsg(false, "Backend '{}' tried to run but Build() is not implemented.",
			    GetName().AsString());
		}
	};
}    // namespace rift
