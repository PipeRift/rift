// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Math/Vector.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CNodePosition : public Struct
	{
		STRUCT(CNodePosition, Struct)

		PROP(position)
		v2 position;

		CNodePosition() = default;
		CNodePosition(v2 position) : position{position} {}
	};
	static void Read(Serl::ReadContext& ct, CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
	static void Write(Serl::WriteContext& ct, const CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
}    // namespace Rift
