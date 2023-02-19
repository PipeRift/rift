// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>

namespace rift::AST
{
	enum class RiftModuleTarget : p::u8
	{
		Executable,
		Shared,
		Static
	};
}    // namespace rift::AST
ENUM(rift::AST::RiftModuleTarget)


namespace rift::AST
{
	static constexpr p::StringView moduleFilename = "__module__.rf";

	struct CModule : public p::Struct
	{
		STRUCT(CModule, p::Struct)

		PROP(target)
		RiftModuleTarget target = RiftModuleTarget::Executable;

		PROP(dependencies)
		p::TArray<p::Tag> dependencies;
	};
}    // namespace rift::AST
