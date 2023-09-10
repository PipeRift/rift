// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

#include <Pipe/Core/Profiler.h>
#include <Pipe/Core/String.h>
#include <Pipe/Reflect/Reflection.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	class Backend;


	struct CompileError : public p::Struct
	{
		P_STRUCT(CompileError, p::Struct)

		P_PROP(text)
		String text;
	};


	struct Compiler : public p::Struct
	{
		P_STRUCT(Compiler, p::Struct)

		AST::Tree& ast;
		CompilerConfig config;
		TArray<CompileError> errors;


	public:
		Compiler(AST::Tree& ast, const CompilerConfig& config) : ast{ast}, config{config} {}

		// Errors
		void Error(StringView str);
		const TArray<CompileError>& GetErrors() const
		{
			return errors;
		}
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};


	void Build(AST::Tree& tree, const CompilerConfig& config, TPtr<Backend> backend);

	void Build(AST::Tree& ast, const CompilerConfig& config, ClassType* backendType);

	template<typename T>
	void Build(AST::Tree& ast, const CompilerConfig& config)
	{
		Build(ast, config, T::GetStaticType());
	}
}    // namespace rift
