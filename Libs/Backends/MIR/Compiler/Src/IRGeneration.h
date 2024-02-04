// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Components.h"

#include <AST/Components/CModule.h>
#include <AST/Components/Expressions.h>
#include <AST/Components/Literals.h>
#include <AST/Components/Statements.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Id.h>
#include <Components/Declarations.h>
#include <Pipe/Core/Set.h>
#include <PipeECS.h>


namespace rift
{
	struct Compiler;
}

namespace rift::MIR
{
	// Defines a single ecs access dfor the entire IR generation
	using MIRAccess = p::TAccessRef<ast::CStmtOutput, ast::CStmtOutputs, ast::CExprInputs,
	    ast::CStmtIf, ast::CExprCallId, ast::CExprTypeId, ast::CExprOutputs, ast::CNamespace,
	    ast::CDeclType, ast::CDeclVariable, ast::CDeclStruct, ast::CDeclClass, ast::CDeclStatic,
	    ast::CParent, ast::CInvalid, ast::CChild, ast::CModule, ast::CLiteralBool,
	    ast::CLiteralIntegral, ast::CLiteralFloating, ast::CLiteralString, ast::CDeclFunction,
	    CDeclCStruct, CDeclCStatic, p::TWrite<CMIRType>, p::TWrite<CMIRFunctionSignature>,
	    p::TWrite<CMIRLiteral>>;

	void GenerateC(Compiler& compiler);


	struct CGenerator
	{
		static const p::TSet<p::Tag> reservedNames;

		Compiler& compiler;
		MIRAccess access;
		p::String* code = nullptr;


		void GenerateModule(ast::Id moduleId);

		void BindNativeTypes();
		void GenerateLiterals();

		void DeclareStructs(p::TView<ast::Id> ids);
		void DefineStructs(p::TView<ast::Id> ids);
		void DeclareFunctions(p::TView<ast::Id> ids, bool useFullName = true);
		void DefineFunctions(p::TView<ast::Id> ids);

		void AddStmtBlock(ast::Id firstStmtId);
		void AddStmtIf(ast::Id id);
		void AddExpr(const ast::ExprOutput& output);
		void AddCall(ast::Id id, const ast::CExprCallId& call);
		void CreateMain(ast::Id functionId);
		ast::Id FindMainFunction(p::TView<ast::Id> functionIds);
	};

}    // namespace rift::MIR
