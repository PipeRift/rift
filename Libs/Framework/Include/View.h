// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CType.h"

#include <Events/Function.h>
#include <Types/Struct.h>


namespace Rift
{
	struct View : public Struct
	{
		STRUCT(View, Struct)

		PROP(name)
		Name name;

		Type supportedTypes = Type::None;

		TFunction<void()> onDrawEditor;
	};
}    // namespace Rift
