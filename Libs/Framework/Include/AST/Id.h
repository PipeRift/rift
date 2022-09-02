// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <Pipe/Core/TypeList.h>
#include <Pipe/ECS/Components/CChild.h>
#include <Pipe/ECS/Components/CParent.h>
#include <Pipe/ECS/Id.h>
#include <Pipe/Reflect/Builders/NativeTypeBuilder.h>
#include <Pipe/Serialize/SerializationFwd.h>


namespace rift::AST
{
	using namespace p;

	using Id          = ecs::Id;
	constexpr Id NoId = ecs::NoId;

	using CParent = ecs::CParent;
	using CChild  = ecs::CChild;
}    // namespace rift::AST

namespace p::ecs
{
	void Read(p::Reader& ct, p::ecs::Id& val);
	void Write(p::Writer& ct, p::ecs::Id val);
}    // namespace p::ecs
