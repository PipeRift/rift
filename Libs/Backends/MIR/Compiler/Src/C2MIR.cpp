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
	void InitCToMIROptions(c2mir_options& options)
	{
		int inclP, ldirP = FALSE; /* to remove an uninitialized warning */

		options.message_file = stderr;
		options.debug_p = options.verbose_p = options.ignore_warnings_p = FALSE;
		options.asm_p = options.object_p = options.no_prepro_p = options.prepro_only_p = FALSE;
		options.syntax_only_p = options.pedantic_p = FALSE;

		options.macro_commands     = nullptr;
		options.macro_commands_num = 0;
	}

	void CToMIR(Compiler& compiler, MIR_context* ctx)
	{
		c2mir_init(ctx);

		c2mir_options options;
		InitCToMIROptions(options);

		for (ast::Id moduleId : FindAllIdsWith<ast::CModule>(compiler.ast))
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
			if (codeLeft->size() > 0)
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
