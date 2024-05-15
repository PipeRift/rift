// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "AST/Components/Declarations.h"

#include <Pipe/Core/Function.h>
#include <PipeReflect.h>


namespace rift
{
	struct View
	{
		P_STRUCT(View)

		P_PROP(name)
		p::Tag name;

		p::TArray<p::Tag> supportedTypes;

		p::TFunction<void()> onDrawEditor;
	};
}    // namespace rift
