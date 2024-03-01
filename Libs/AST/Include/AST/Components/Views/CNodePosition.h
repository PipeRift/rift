// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>
#include <PipeVectors.h>


namespace rift
{
	struct CNodePosition : public p::Struct
	{
		P_STRUCT(CNodePosition, p::Struct)

		P_PROP(position)
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
