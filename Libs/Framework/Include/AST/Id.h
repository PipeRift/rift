// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Platform.h>
#include <Core/TypeList.h>
#include <ECS/Id.h>
#include <Reflection/Builders/NativeTypeBuilder.h>
#include <Serialization/SerializationFwd.h>



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
