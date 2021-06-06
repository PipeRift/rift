// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Object/Struct.h>


namespace Rift
{
	struct CEditorNode : public Struct
	{
		STRUCT(CEditorNode, Struct)

		PROP(v2, position)
		v2 position;
	};
}    // namespace Rift
