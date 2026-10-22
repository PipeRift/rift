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

		Expect(ast.Get<const ast::CGeneratedName>(globalA).name).ToEqual(p::Tag{"a"});
		Expect(ast.Get<const ast::CGeneratedName>(paramA).name).ToEqual(p::Tag{"a_1"});
		Expect(ast.Get<const ast::CGeneratedName>(paramC).name).ToEqual(p::Tag{"c"});
		Expect(ast.Get<const ast::CGeneratedName>(localA).name).ToEqual(p::Tag{"a_2"});
		Expect(ast.Get<const ast::CGeneratedName>(localB).name).ToEqual(p::Tag{"b"});
		Expect(ast.Get<const ast::CGeneratedName>(localC).name).ToEqual(p::Tag{"c_1"});

		Expect(ast.Get<const ast::CGeneratedName>(local2A).name).ToEqual(p::Tag{"a_1"});
		Expect(ast.Get<const ast::CGeneratedName>(local2B).name).ToEqual(p::Tag{"b"});
		Expect(ast.Get<const ast::CGeneratedName>(local2C).name).ToEqual(p::Tag{"c"});
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

		Expect(ast.Get<const ast::CGeneratedName>(global1).name).ToEqual(p::Tag{"x"});
		Expect(ast.Get<const ast::CGeneratedName>(global2).name).ToEqual(p::Tag{"x_1"});
		Expect(ast.Get<const ast::CGeneratedName>(param1).name).ToEqual(p::Tag{"p"});
		Expect(ast.Get<const ast::CGeneratedName>(param2).name).ToEqual(p::Tag{"p_1"});
		Expect(ast.Get<const ast::CGeneratedName>(local1).name).ToEqual(p::Tag{"l"});
		Expect(ast.Get<const ast::CGeneratedName>(local2).name).ToEqual(p::Tag{"l_1"});
	});

	It("Avoids collisions between suffixed base names", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id localX  = AddLocal(ast, functionId, p::Tag{"x"});
		ast::Id localX1 = AddLocal(ast, functionId, p::Tag{"x_1"});
		ast::Id localX2 = AddLocal(ast, functionId, p::Tag{"x"});

		AssignNames(ast);

		Expect(ast.Get<const ast::CGeneratedName>(localX).name).ToEqual(p::Tag{"x"});
		Expect(ast.Get<const ast::CGeneratedName>(localX1).name).ToEqual(p::Tag{"x_1"});
		Expect(ast.Get<const ast::CGeneratedName>(localX2).name).ToEqual(p::Tag{"x_2"});
	});

	It("Avoids collisions with generated suffixed names", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id localX    = AddLocal(ast, functionId, p::Tag{"x"});
		ast::Id localX1   = AddLocal(ast, functionId, p::Tag{"x"});
		ast::Id localX1_1 = AddLocal(ast, functionId, p::Tag{"x_1"});

		AssignNames(ast);

		Expect(ast.Get<const ast::CGeneratedName>(localX).name).ToEqual(p::Tag{"x"});
		Expect(ast.Get<const ast::CGeneratedName>(localX1).name).ToEqual(p::Tag{"x_1"});
		Expect(ast.Get<const ast::CGeneratedName>(localX1_1).name).ToEqual(p::Tag{"x_1_1"});
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
		Expect(ast.Get<const ast::CGeneratedName>(index1).name).ToEqual(p::Tag{"index"});
		Expect(ast.Get<const ast::CGeneratedName>(index2).name).ToEqual(p::Tag{"index_1"});

		const ast::Id end1 = ast.Get<ast::CExprInputs>(for1).pinIds[1];
		const ast::Id end2 = ast.Get<ast::CExprInputs>(for2).pinIds[1];
		Expect(ast.Get<const ast::CGeneratedName>(end1).name).ToEqual(p::Tag{"end"});
		Expect(ast.Get<const ast::CGeneratedName>(end2).name).ToEqual(p::Tag{"end_1"});

		for (p::Id pinId : ast.Get<const ast::CExprInputs>(for1).pinIds)
		{
			Expect(ast.Has<ast::CGeneratedName>(pinId)).ToBeTrue();
		}
		for (p::Id pinId : ast.Get<const ast::CExprInputs>(for2).pinIds)
		{
			Expect(ast.Has<ast::CGeneratedName>(pinId)).ToBeTrue();
		}
	});

	It("Assigns names to statement input pins", []()
	{
		ast::Tree ast;

		ast::Id typeId     = ast::CreateType(ast, ASTModule::staticType, p::Tag{"TestStatic"});
		ast::Id functionId = ast::AddFunction({ast, typeId}, p::Tag{"Function"});

		ast::Id ifId = ast::AddIf({ast, typeId});
		ast::TryConnectStmt(ast, functionId, ifId);

		AssignNames(ast);

		const ast::Id conditionPin = ast.Get<const ast::CExprInputs>(ifId).pinIds[0];
		Expect(ast.Has<ast::CGeneratedName>(conditionPin)).ToBeTrue();
	});
});
