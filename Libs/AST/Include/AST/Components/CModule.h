// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include <Pipe/Files/Paths.h>
#include <PipeReflect.h>


namespace rift::ast
{
	enum class RiftModuleTarget : p::u8
	{
		Executable,
		Shared,
		Static
	};
}    // namespace rift::ast
P_ENUM(rift::ast::RiftModuleTarget)


namespace rift::ast
{
	static constexpr p::StringView moduleFilename = "__module__.rf";

	struct CModule
	{
		P_STRUCT(CModule)

		P_PROP(target)
		RiftModuleTarget target = RiftModuleTarget::Executable;

		P_PROP(dependencies, p::PF_Edit)
		p::TArray<p::Tag> dependencies;
	};
}    // namespace rift::ast
