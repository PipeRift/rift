// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"
#include "Project.h"

#include <CoreObject.h>
#include <Profiler.h>
#include <Reflection/Reflection.h>


namespace Rift::Compiler
{
	struct CompileError : public Struct
	{
		STRUCT(CompileError, Struct)

		PROP(String, text)
		String text;
	};


	struct Context : public Struct
	{
		STRUCT(Context, Struct)

		PROP(Config, config)
		Config config;

		TPtr<Project> project;

		PROP(TArray<CompileError>, errors)
		TArray<CompileError> errors;


		void AddError(StringView str);
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};
}    // namespace Rift::Compiler
