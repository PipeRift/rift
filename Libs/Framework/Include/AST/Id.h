// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <ECS/Id.h>
#include <Platform/Platform.h>
#include <Reflection/Builders/NativeTypeBuilder.h>
#include <Serialization/ContextsFwd.h>
#include <Templates/TypeList.h>


namespace Rift::AST
{
	using Id          = ECS::Id;
	constexpr Id NoId = ECS::NoId;
}    // namespace Rift::AST

namespace Rift::Serl
{
	void Read(ReadContext& ct, AST::Id& val);
	void Write(WriteContext& ct, AST::Id val);
}    // namespace Rift::Serl
