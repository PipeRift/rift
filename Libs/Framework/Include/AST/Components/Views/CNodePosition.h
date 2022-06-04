// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Math/Vector.h>
#include <Reflection/Struct.h>


namespace Rift
{
	struct CNodePosition : public Pipe::Struct
	{
		STRUCT(CNodePosition, Pipe::Struct)

		PROP(position)
		v2 position;

		CNodePosition() = default;
		CNodePosition(v2 position) : position{position} {}
	};
	static void Read(Pipe::ReadContext& ct, CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
	static void Write(Pipe::WriteContext& ct, const CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
}    // namespace Rift
