// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	using namespace Pipe;

	enum class ModuleTarget : u8
	{
		Executable,
		Shared,
		Static
	};
	// TODO: Simplify enum reflection so that ENUM() is not needed

	enum class ModuleType : u8
	{
		Rift,
		CBinding
	};
}    // namespace Rift
// TODO: Simplify enum reflection so that ENUM() is not needed
ENUM(Rift::ModuleTarget)
ENUM(Rift::ModuleType)


namespace Rift
{
	struct CModule : public Pipe::Struct
	{
		STRUCT(CModule, Pipe::Struct)

		PROP(target)
		ModuleTarget target = ModuleTarget::Executable;

		PROP(type)
		ModuleType type = ModuleType::Rift;

		PROP(dependencies)
		TArray<Name> dependencies;

		PROP(libraryFiles)
		TArray<Path> libraryFiles;


		CModule() {}
	};
}    // namespace Rift
