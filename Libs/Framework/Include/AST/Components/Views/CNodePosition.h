// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Math/Vector.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CNodePosition : public p::Struct
	{
		STRUCT(CNodePosition, p::Struct)

		PROP(position)
		p::v2 position;

		CNodePosition() = default;
		CNodePosition(p::v2 position) : position{position} {}
	};
	static void Read(p::Reader& ct, CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
	static void Write(p::Writer& ct, const CNodePosition& val)
	{
		ct.Serialize(val.position);
	}
}    // namespace rift
