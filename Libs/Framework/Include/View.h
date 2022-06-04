// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CType.h"

#include <Core/Function.h>
#include <Reflection/Struct.h>


namespace Rift
{
	struct View : public Pipe::Struct
	{
		STRUCT(View, Pipe::Struct)

		PROP(name)
		Name name;

		Type supportedTypes = Type::None;

		TFunction<void()> onDrawEditor;
	};
}    // namespace Rift
