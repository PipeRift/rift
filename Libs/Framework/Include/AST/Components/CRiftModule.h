// Copyright 2015-2022 Piperift - All rights reserved
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
	struct CRiftModule : public p::Struct
	{
		STRUCT(CRiftModule, p::Struct)

		PROP(target)
		RiftModuleTarget target = RiftModuleTarget::Executable;
	};
}    // namespace rift::AST
