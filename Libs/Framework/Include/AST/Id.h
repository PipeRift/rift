// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Platform.h>
#include <ECS/Id.h>
#include <Reflection/Builders/NativeTypeBuilder.h>
#include <Serialization/ContextsFwd.h>
#include <Templates/TypeList.h>


namespace Rift::AST
{
	using namespace Pipe;

	using Id          = ECS::Id;
	constexpr Id NoId = ECS::NoId;
}    // namespace Rift::AST

namespace Pipe::ECS
{
	void Read(Pipe::ReadContext& ct, Pipe::ECS::Id& val);
	void Write(Pipe::WriteContext& ct, Pipe::ECS::Id val);
}    // namespace Pipe::ECS
