// Copyright 2015-2023 Piperift - All rights reserved

#include "IRGeneration.h"

#include "mir.h"

#include <AST/Id.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <Components/CDeclCStatic.h>
#include <Components/CDeclCStruct.h>


namespace rift::MIR
{
	void GenerateIR(Compiler& compiler, MIR_context_t& ctx)
	{
		MIRAccess access{compiler.ast};
		BindNativeTypes(ctx, access);

		for (AST::Id moduleId : FindAllIdsWith<AST::CModule>(access))
		{
			GenerateIRModule(compiler, access, moduleId, ctx);
		}
	}

	void GenerateIRModule(
	    Compiler& compiler, MIRAccess access, AST::Id moduleId, MIR_context_t& ctx)
	{
		ZoneScoped;
		auto& ast = compiler.ast;

		const Tag name = AST::GetModuleName(compiler.ast, moduleId);

		const auto& module = compiler.ast.Get<const AST::CModule>(moduleId);
		compiler.ast.Add<CMIRModule>(moduleId, MIR_new_module(ctx, name.AsString().data()));

		// NOTE: Module generation here

		MIR_finish_module(ctx);
	}


	void BindNativeTypes(MIR_context_t& ctx, MIRAccess access)
	{
		const auto& nativeTypes = static_cast<AST::Tree&>(access.GetContext()).GetNativeTypes();
		access.Add(nativeTypes.boolId, CMIRType{MIR_T_I8});
		access.Add(nativeTypes.floatId, CMIRType{MIR_T_F});
		access.Add(nativeTypes.doubleId, CMIRType{MIR_T_D});
		access.Add(nativeTypes.u8Id, CMIRType{MIR_T_U8});
		access.Add(nativeTypes.i8Id, CMIRType{MIR_T_I8});
		access.Add(nativeTypes.u16Id, CMIRType{MIR_T_U16});
		access.Add(nativeTypes.i16Id, CMIRType{MIR_T_I16});
		access.Add(nativeTypes.u32Id, CMIRType{MIR_T_U32});
		access.Add(nativeTypes.i32Id, CMIRType{MIR_T_I32});
		access.Add(nativeTypes.u64Id, CMIRType{MIR_T_U64});
		access.Add(nativeTypes.i64Id, CMIRType{MIR_T_I64});
		// access.Add<CIRType>(nativeTypes.stringId, {});
	}
}    // namespace rift::MIR
