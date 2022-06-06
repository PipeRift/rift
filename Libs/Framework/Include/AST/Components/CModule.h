// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	using namespace p;

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
}    // namespace rift
// TODO: Simplify enum reflection so that ENUM() is not needed
ENUM(rift::ModuleTarget)
ENUM(rift::ModuleType)


namespace rift
{
	struct CModule : public p::Struct
	{
		STRUCT(CModule, p::Struct)

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
}    // namespace rift
