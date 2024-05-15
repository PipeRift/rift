// Copyright 2015-2024 Piperift - All rights reserved
#pragma once


#include <PipeReflect.h>
#include <PipeVectors.h>


namespace rift
{
	struct CNodePosition
	{
		P_STRUCT(CNodePosition)

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
