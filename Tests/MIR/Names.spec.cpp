// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Components/CGeneratedName.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Statements.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TypeUtils.h>
#include <ASTModule.h>
#include <Compiler/Compiler.h>
#include <IRGeneration.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


static void AssignNames(ast::Tree& ast)
{
	CompilerConfig config{};
	Compiler compiler{ast, config};
	MIR::MIRScope scope{ast};
	MIR::CGenerator generator{compiler, scope};
	generator.AssignGeneratedNames();
}

static ast::Id AddLocal(ast::Tree& ast, ast::Id ownerId, p::Tag name)
{
	ast::Id id = p::AddId(ast);
	ast.Add<ast::CNamespace>(id, name);
	ast.Add<ast::CDeclVariable>(id);
	p::AttachId(ast, ownerId, id);
	return id;
}

P_SPEC("MIR.Names", []()
{
	It("Assigns names avoiding global and function scopes", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id globalA    = ast::AddVariable({ast, typeId}, p::Tag{"a"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id paramA =
		    ast::AddFunctionInput(ast, functionId, p::Tag{"a"}, ast.GetNativeTypes().i32Id);
		ast::Id paramC =
		    ast::AddFunctionInput(ast, functionId, p::Tag{"c"}, ast.GetNativeTypes().i32Id);

		ast::Id localA = AddLocal(ast, functionId, p::Tag{"a"});
		ast::Id localB = AddLocal(ast, functionId, p::Tag{"b"});
		ast::Id localC = AddLocal(ast, functionId, p::Tag{"c"});

		// A second function starts with a fresh local scope and only inherits globals
		ast::Id function2 = ast::AddFunction({ast, typeId}, p::Tag{"Function2"});
		ast::Id local2A   = AddLocal(ast, function2, p::Tag{"a"});
		ast::Id local2B   = AddLocal(ast, function2, p::Tag{"b"});
		ast::Id local2C   = AddLocal(ast, function2, p::Tag{"c"});

		AssignNames(ast);

		Expect(ast::GetGeneratedName(ast, globalA)).ToEqual(p::Tag{"a"});
		Expect(ast::GetGeneratedName(ast, paramA)).ToEqual(p::Tag{"a_1"});
		Expect(ast::GetGeneratedName(ast, paramC)).ToEqual(p::Tag{"c"});
		Expect(ast::GetGeneratedName(ast, localA)).ToEqual(p::Tag{"a_2"});
		Expect(ast::GetGeneratedName(ast, localB)).ToEqual(p::Tag{"b"});
		Expect(ast::GetGeneratedName(ast, localC)).ToEqual(p::Tag{"c_1"});

		Expect(ast::GetGeneratedName(ast, local2A)).ToEqual(p::Tag{"a_1"});
		Expect(ast::GetGeneratedName(ast, local2B)).ToEqual(p::Tag{"b"});
		Expect(ast::GetGeneratedName(ast, local2C)).ToEqual(p::Tag{"c"});
	});

	It("Assigns unique names to same-scope declarations", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id global1    = ast::AddVariable({ast, typeId}, p::Tag{"x"});
		ast::Id global2    = ast::AddVariable({ast, typeId}, p::Tag{"x"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id param1 =
		    ast::AddFunctionInput(ast, functionId, p::Tag{"p"}, ast.GetNativeTypes().i32Id);
		ast::Id param2 =
		    ast::AddFunctionInput(ast, functionId, p::Tag{"p"}, ast.GetNativeTypes().i32Id);

		ast::Id local1 = AddLocal(ast, functionId, p::Tag{"l"});
		ast::Id local2 = AddLocal(ast, functionId, p::Tag{"l"});

		AssignNames(ast);

		Expect(ast::GetGeneratedName(ast, global1)).ToEqual(p::Tag{"x"});
		Expect(ast::GetGeneratedName(ast, global2)).ToEqual(p::Tag{"x_1"});
		Expect(ast::GetGeneratedName(ast, param1)).ToEqual(p::Tag{"p"});
		Expect(ast::GetGeneratedName(ast, param2)).ToEqual(p::Tag{"p_1"});
		Expect(ast::GetGeneratedName(ast, local1)).ToEqual(p::Tag{"l"});
		Expect(ast::GetGeneratedName(ast, local2)).ToEqual(p::Tag{"l_1"});
	});

	It("Assigns unique names to for loop indices", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id for1 = ast::AddFor({ast, typeId});
		ast::Id for2 = ast::AddFor({ast, typeId});
		ast::TryConnectStmt(ast, functionId, for1);
		ast::TryConnectStmt(ast, ast.Get<ast::CStmtOutputs>(for1).pinIds[1], for2);

		AssignNames(ast);

		const ast::Id index1 = ast.Get<ast::CExprOutputs>(for1).pinIds[0];
		const ast::Id index2 = ast.Get<ast::CExprOutputs>(for2).pinIds[0];
		Expect(ast::GetGeneratedName(ast, index1)).ToEqual(p::Tag{"index"});
		Expect(ast::GetGeneratedName(ast, index2)).ToEqual(p::Tag{"index_1"});
	});
});
