// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Math/Vector.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CGraphTransform : public Struct
	{
		STRUCT(CGraphTransform, Struct)

		PROP(position)
		v2 position;

		CGraphTransform() {}
		CGraphTransform(v2 position) : position(position) {}
	};
}    // namespace Rift
