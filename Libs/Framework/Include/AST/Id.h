// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <Pipe/Core/TypeList.h>
#include <Pipe/ECS/Components/CChild.h>
#include <Pipe/ECS/Components/CParent.h>
#include <Pipe/PipeECS.h>
#include <Pipe/Reflect/Builders/NativeTypeBuilder.h>
#include <Pipe/Serialize/SerializationFwd.h>


namespace rift::AST
{
	using namespace p;
	using Id          = p::Id;
	constexpr Id NoId = p::NoId;

	using CParent = p::ecs::CParent;
	using CChild  = p::ecs::CChild;
}    // namespace rift::AST
