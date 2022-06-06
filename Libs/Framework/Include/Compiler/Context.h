// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

#include <PCore/Profiler.h>
#include <PRefl/Reflection.h>
#include <PRefl/Struct.h>


namespace rift::Compiler
{
	struct CompileError : public p::Struct
	{
		STRUCT(CompileError, p::Struct)

		PROP(text)
		String text;
	};


	struct Context : public p::Struct
	{
		STRUCT(Context, p::Struct)

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
}    // namespace rift::Compiler
