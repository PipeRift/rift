// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Math/Vector.h>
#include <Object/Struct.h>


namespace Rift
{
	struct CGraphTransform : public Struct
	{
		STRUCT(CGraphTransform, Struct)

		PROP(position)
		v2 position;
	};
}    // namespace Rift
