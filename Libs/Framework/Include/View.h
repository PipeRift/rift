// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CType.h"

#include <Pipe/Core/Function.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct View : public p::Struct
	{
		STRUCT(View, p::Struct)

		PROP(name)
		Name name;

		RiftType supportedTypes = RiftType::None;

		TFunction<void()> onDrawEditor;
	};
}    // namespace rift
