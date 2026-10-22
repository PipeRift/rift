// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Components/Literals.h>
#include <AST/Tree.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Strings", []()
{
	It("Has a native type for string views", []()
	{
		ast::Tree ast;

		const auto& natives = ast.GetNativeTypes();
		Expect(ast.IsValid(natives.stringId)).ToBeTrue();
		Expect(ast.IsValid(natives.stringViewId)).ToBeTrue();

		const auto* stringName = ast.TryGet<const ast::CNamespace>(natives.stringId);
		const auto* viewName   = ast.TryGet<const ast::CNamespace>(natives.stringViewId);
		Expect(stringName != nullptr).ToBeTrue();
		Expect(viewName != nullptr).ToBeTrue();
		Expect(stringName->name == p::Tag{"String"}).ToBeTrue();
		Expect(viewName->name == p::Tag{"StringView"}).ToBeTrue();
	});

	It("Creates string literals as string views", []()
	{
		ast::Tree ast;

		const auto& natives = ast.GetNativeTypes();
		ast::Id id          = ast::AddLiteral({ast, ast::NoId}, natives.stringViewId);

		Expect(ast.IsValid(id)).ToBeTrue();
		Expect(ast.Has<ast::CLiteralString>(id)).ToBeTrue();
		Expect(ast.Get<ast::CExprTypeId>(id).id).ToEqual(natives.stringViewId);
	});

	It("Does not create literals for owned strings", []()
	{
		ast::Tree ast;

		const auto& natives = ast.GetNativeTypes();
		Expect(ast::CanCreateLiteral(ast, natives.stringId)).ToBeFalse();
		Expect(ast::CanCreateLiteral(ast, natives.stringViewId)).ToBeTrue();

		ast::Id id = ast::AddLiteral({ast, ast::NoId}, natives.stringId);
		Expect(ast.IsValid(id)).ToBeFalse();
	});
});
