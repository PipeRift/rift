// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CDeclType.h"

#include <Pipe/Core/Function.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct View : public p::Struct
	{
		P_STRUCT(View, p::Struct)

		P_PROP(name)
		p::Tag name;

		p::TArray<p::Tag> supportedTypes;

		p::TFunction<void()> onDrawEditor;
	};
}    // namespace rift
