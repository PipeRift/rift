// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

#include <Profiler.h>
#include <Reflection/Reflection.h>
#include <Reflection/Struct.h>


namespace Rift::Compiler
{
	struct CompileError : public Pipe::Struct
	{
		STRUCT(CompileError, Pipe::Struct)

		PROP(text)
		String text;
	};


	struct Context : public Pipe::Struct
	{
		STRUCT(Context, Pipe::Struct)

		AST::Tree& ast;
		Config config;
		TArray<CompileError> errors;


	public:
		Context(AST::Tree& ast, const Config& config) : ast{ast}, config{config} {}

		// Errors
		void AddError(StringView str);
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};
}    // namespace Rift::Compiler
