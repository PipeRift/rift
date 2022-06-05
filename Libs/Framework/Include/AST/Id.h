// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Platform.h>
#include <ECS/Id.h>
#include <Reflection/Builders/NativeTypeBuilder.h>
#include <Serialization/ContextsFwd.h>
#include <Templates/TypeList.h>


namespace rift::AST
{
	using namespace pipe;

	using Id          = ECS::Id;
	constexpr Id NoId = ECS::NoId;
}    // namespace rift::AST

namespace pipe::ECS
{
	void Read(pipe::ReadContext& ct, pipe::ECS::Id& val);
	void Write(pipe::WriteContext& ct, pipe::ECS::Id val);
}    // namespace pipe::ECS
