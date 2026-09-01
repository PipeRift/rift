// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Compiler/CompilerConfig.h"

#include <PipeReflect.h>
#include <PipeStrings.h>


namespace rift
{
	class Backend;


	struct CompileError
	{
		P_STRUCT(CompileError)

		P_PROP(text)
		p::String text;
	};


	struct Compiler
	{
		P_STRUCT(Compiler)

		ast::Tree& ast;
		CompilerConfig config;
		p::TArray<CompileError> errors;


	public:
		Compiler(ast::Tree& ast, const CompilerConfig& config) : ast{ast}, config{config} {}

		// Errors
		void Error(p::StringView str);
		const p::TArray<CompileError>& GetErrors() const
		{
			return errors;
		}
		bool HasErrors() const
		{
			return errors.Size() > 0;
		}
	};


	void Build(ast::Tree& tree, const CompilerConfig& config, p::TPtr<Backend> backend);

	void Build(ast::Tree& ast, const CompilerConfig& config, p::TypeId backendType);

	template<typename T>
	void Build(ast::Tree& ast, const CompilerConfig& config)
	{
		Build(ast, config, p::GetTypeId<T>());
	}
}    // namespace rift
