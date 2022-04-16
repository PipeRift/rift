// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Types/Struct.h>


namespace Rift
{
	enum class ModuleTarget : u8
	{
		Executable,
		Shared,
		Static
	};
	// TODO: Simplify enum reflection so that ENUM() is not needed
	ENUM(ModuleTarget)

	enum class ModuleType : u8
	{
		Rift,
		CBinding
	};
	// TODO: Simplify enum reflection so that ENUM() is not needed
	ENUM(ModuleType)


	struct CModule : public Struct
	{
		STRUCT(CModule, Struct)

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
