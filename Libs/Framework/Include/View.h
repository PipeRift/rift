// Copyright 2015-2023 Piperift - All rights reserved
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
		p::Name name;

		AST::RiftType supportedTypes = AST::RiftType::None;

		p::TFunction<void()> onDrawEditor;
	};
}    // namespace rift
