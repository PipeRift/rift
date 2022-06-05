// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Math/Vector.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct CNodePosition : public pipe::Struct
	{
		STRUCT(CNodePosition, pipe::Struct)

		PROP(position)
		v2 position;

		CNodePosition() = default;
		CNodePosition(v2 position) : position{position} {}
	};
	static void Read(pipe::ReadContext& ct, CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
	static void Write(pipe::WriteContext& ct, const CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
}    // namespace rift
