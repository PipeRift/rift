// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Project.h"
#include "Reflection/Reflection.h"

#include <CoreObject.h>
#include <Profiler.h>


namespace Rift::Backends
{
	struct CompilerConfig : public Struct
	{
		STRUCT(CompilerConfig, Struct)

		Path buildPath;
		Path intermediatesPath;
		Path binariesPath;


		void Init(TPtr<Project> rootProject);
	};


	struct CompileError : public Struct
	{
		STRUCT(CompileError, Struct)

		PROP(String, text)
		String text;
	};


	struct CompilerContext : public Struct
	{
		STRUCT(CompilerContext, Struct)

		PROP(CompilerConfig, config)
		CompilerConfig config;

		TPtr<Project> project;

		PROP(TArray<CompileError>, errors)
		TArray<CompileError> errors;


		void AddError(StringView str);
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};
}    // namespace Rift::Backends
