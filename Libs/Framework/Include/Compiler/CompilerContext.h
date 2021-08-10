// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

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

		AST::Tree& ast;
		Config config;
		TArray<CompileError> errors;


		Context(AST::Tree& ast, const Config& config) : ast{ast}, config{config} {}

		// Errors
		void AddError(StringView str);
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};
}    // namespace Rift::Compiler
