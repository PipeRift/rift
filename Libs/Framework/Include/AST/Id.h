// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <PCore/Platform.h>
#include <PCore/TypeList.h>
#include <PECS/Id.h>
#include <PRefl/Builders/NativeTypeBuilder.h>
#include <PSerl/SerializationFwd.h>


namespace rift::AST
{
	using namespace p;

	using Id          = ecs::Id;
	constexpr Id NoId = ecs::NoId;
}    // namespace rift::AST

namespace p::ecs
{
	void Read(p::Reader& ct, p::ecs::Id& val);
	void Write(p::Writer& ct, p::ecs::Id val);
}    // namespace p::ecs
