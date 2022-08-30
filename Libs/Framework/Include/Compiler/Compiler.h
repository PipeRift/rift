// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

#include <Pipe/Core/Profiler.h>
#include <Pipe/Core/String.h>
#include <Pipe/Reflect/Reflection.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::compiler
{
	class Backend;


	struct CompileError : public p::Struct
	{
		STRUCT(CompileError, p::Struct)

		PROP(text)
		String text;
	};


	struct Compiler : public p::Struct
	{
		STRUCT(Compiler, p::Struct)

		AST::Tree& ast;
		Config config;
		TArray<CompileError> errors;


	public:
		Compiler(AST::Tree& ast, const Config& config) : ast{ast}, config{config} {}

		// Errors
		void AddError(StringView str);
		const TArray<CompileError>& GetErrors() const
		{
			return errors;
		}
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};


	void Build(AST::Tree& tree, const Config& config, TPtr<Backend> backend);

	void Build(AST::Tree& ast, const Config& config, ClassType* backendType);

	template<typename T>
	void Build(AST::Tree& ast, const Config& config)
	{
		Build(ast, config, T::GetStaticType());
	}
}    // namespace rift::compiler
