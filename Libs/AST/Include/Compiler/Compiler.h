// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Compiler/CompilerConfig.h"

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
		p::String text;
	};


	struct Compiler : public p::Struct
	{
		P_STRUCT(Compiler, p::Struct)

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

	void Build(ast::Tree& ast, const CompilerConfig& config, p::ClassType* backendType);

	template<typename T>
	void Build(ast::Tree& ast, const CompilerConfig& config)
	{
		Build(ast, config, T::GetStaticType());
	}
}    // namespace rift
