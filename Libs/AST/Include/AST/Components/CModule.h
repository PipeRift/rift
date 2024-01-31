// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/Struct.h>

namespace rift::ast
{
	enum class RiftModuleTarget : p::u8
	{
		Executable,
		Shared,
		Static
	};
}    // namespace rift::ast
ENUM(rift::ast::RiftModuleTarget)


namespace rift::ast
{
	static constexpr p::StringView moduleFilename = "__module__.rf";

	struct CModule : public p::Struct
	{
		P_STRUCT(CModule, p::Struct)

		P_PROP(target)
		RiftModuleTarget target = RiftModuleTarget::Executable;

		P_PROP(dependencies)
		p::TArray<p::Tag> dependencies;
	};
}    // namespace rift::ast
