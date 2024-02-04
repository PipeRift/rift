// Copyright 2015-2023 Piperift - All rights reserved

#include "C2MIR.h"

#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
extern "C"
{
#include <c2mir/c2mir.h>
}

namespace rift::MIR
{
	struct OptionsData
	{
		TArray<const char*> headers;
		TArray<c2mir_macro_command> definitions;
	};

	void InitCToMIROptions(
	    const CompilerConfig& config, OptionsData& optionsData, c2mir_options& options)
	{
		// Fill defaults
		options.message_file       = stderr;
		options.debug_p            = false;
		options.verbose_p          = false;
		options.ignore_warnings_p  = false;
		options.no_prepro_p        = false;
		options.prepro_only_p      = false;
		options.syntax_only_p      = false;
		options.pedantic_p         = false;
		options.asm_p              = false;
		options.object_p           = false;
		options.module_num         = 0;
		options.prepro_output_file = nullptr;
		options.output_file_name   = nullptr;

		options.macro_commands_num = 0;
		options.macro_commands     = nullptr;

		options.include_dirs_num = 0;
		options.include_dirs     = nullptr;

		// Fill from config
		// options.debug_p   = config.debug;
		options.verbose_p = config.verbose;

		if (options.output_file_name == nullptr && options.prepro_only_p)
		{
			options.prepro_output_file = stdout;
		}

		// TODO: Fill headers. No headers needed for now
		// TODO: Fill definitions. No definitions needed for now

		options.include_dirs_num   = optionsData.headers.Size();
		options.include_dirs       = optionsData.headers.Data();
		options.macro_commands_num = optionsData.definitions.Size();
		options.macro_commands     = optionsData.definitions.Data();
	}

	void CToMIR(Compiler& compiler, MIR_context* ctx)
	{
		c2mir_init(ctx);

		auto moduleIds = p::FindAllIdsWith<ast::CModule>(compiler.ast);

		OptionsData optionsData;
		c2mir_options options;
		InitCToMIROptions(compiler.config, optionsData, options);

		for (ast::Id moduleId : moduleIds)
		{
			p::Tag name     = ast::GetModuleName(compiler.ast, moduleId);
			auto& mirModule = compiler.ast.Get<CMIRModule>(moduleId);
			CToMIRModule(compiler, ctx, options, name, mirModule);
		}

		c2mir_finish(ctx);
	}

	void CToMIRModule(Compiler& compiler, MIR_context* ctx, c2mir_options& options, p::Tag name,
	    const CMIRModule& module)
	{
		auto getCode = [](void* data) -> p::i32 {
			auto* codeLeft = static_cast<StringView*>(data);
			if (codeLeft->size() > -1)
			{
				*codeLeft = Strings::RemoveFromStart(*codeLeft, 1);
				return *codeLeft->data();
			}
			return EOF;
		};

		StringView code = module.code;
		if (!c2mir_compile(ctx, &options, getCode, &code, name.Data(), nullptr))
		{
			compiler.Error("C to MIR compilation failed");
		}
	}
}    // namespace rift::MIR
