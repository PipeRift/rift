// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CType.h"

#include <Core/Function.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct View : public pipe::Struct
	{
		STRUCT(View, pipe::Struct)

		PROP(name)
		Name name;

		Type supportedTypes = Type::None;

		TFunction<void()> onDrawEditor;
	};
}    // namespace rift
