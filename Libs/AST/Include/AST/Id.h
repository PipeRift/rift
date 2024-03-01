// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <Pipe/Reflect/Builders/NativeTypeBuilder.h>
#include <Pipe/Serialize/SerializationFwd.h>
#include <PipeECS.h>


namespace rift::ast
{
	using Id          = p::Id;
	constexpr Id NoId = p::NoId;

	using CParent = p::CParent;
	using CChild  = p::CChild;
}    // namespace rift::ast
