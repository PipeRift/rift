// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <AST/Components/CNamespace.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/TypeUtils.h>
#include <ASTModule.h>
#include <PipeTest.h>


using namespace p;
using namespace rift;


P_SPEC("AST.Namespaces", []()
{
	It("Can get namespaces", []()
	{
		ast::Tree ast;

		ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
		Expect(ast::GetNamespace(ast, functionId).ToString().c_str()).ToEqual("TestFunction");
		Expect(ast::GetParentNamespace(ast, functionId).ToString().c_str()).ToEqual("");

		ast::Id classBId    = ast::CreateType(ast, ASTModule::classType, "TestClass");
		ast::Id functionBId = ast::AddFunction({ast, classBId}, "TestFunction");
		Expect(ast::GetNamespace(ast, functionBId).ToString().c_str())
		    .ToEqual("TestClass.TestFunction");
		Expect(ast::GetParentNamespace(ast, functionBId).ToString().c_str()).ToEqual("TestClass");

		ast::Id parentC = p::AddId(ast);
		ast.Add<ast::CModule>(parentC);
		ast.Add(parentC, ast::CNamespace{"SomeScope"});
		ast::Id classCId = ast::CreateType(ast, ASTModule::classType, "TestClass");
		p::AttachId(ast, parentC, classCId);
		ast::Id functionCId = ast::AddFunction({ast, classCId}, "TestFunction");
		Expect(ast::GetNamespace(ast, functionCId).ToString().c_str())
		    .ToEqual("SomeScope.TestClass.TestFunction");
		Expect(ast::GetParentNamespace(ast, functionCId).ToString().c_str())
		    .ToEqual("SomeScope.TestClass");
	});

	It("Can get local namespaces", []()
	{
		ast::Tree ast;

		ast::Id parent = p::AddId(ast);
		ast.Add<ast::CModule>(parent);
		ast.Add(parent, ast::CNamespace{"SomeModule"});
		ast::Id classId = ast::CreateType(ast, ASTModule::classType, "TestClass");
		p::AttachId(ast, parent, classId);
		ast::Id functionId = ast::AddFunction({ast, classId}, "TestFunction");
		p::String ns       = ast::GetNamespace(ast, functionId).ToString(true);
		Expect(ns.c_str()).ToEqual("#TestClass.TestFunction");
	});

	It("Can initialize", []()
	{
		ast::Namespace ns0{};
		Expect(ns0.scopes[0].IsNone()).ToBeTrue();
		Expect(ns0.Size()).ToEqual(0);
		Expect(ns0.IsEmpty()).ToBeTrue();

		ast::Namespace ns1{"A"};
		Expect(ns1.scopes[0].AsString().data()).ToEqual("A");
		Expect(ns1.scopes[1].IsNone()).ToBeTrue();
		Expect(ns1.Size()).ToEqual(1);
		Expect(ns1.IsEmpty()).ToBeFalse();

		ast::Namespace ns2{"A", "B"};
		Expect(ns2.scopes[0].AsString().data()).ToEqual("A");
		Expect(ns2.scopes[1].AsString().data()).ToEqual("B");
		Expect(ns2.scopes[2].IsNone()).ToBeTrue();
		Expect(ns2.Size()).ToEqual(2);
		Expect(ns2.IsEmpty()).ToBeFalse();
	});

	It("Can iterate", []()
	{
		ast::Namespace ns0{};
		for (const Tag& name : ns0)
		{
			Expect(false).ToBeTrue();
		}

		ast::Namespace ns1{"C"};
		for (const Tag& name : ns1)
		{
			Expect(name.AsString().data()).ToEqual("C");
		}

		ast::Namespace ns2{"A", "B"};
		i32 i = 0;
		for (const Tag& name : ns2)
		{
			Expect(name.AsString().data()).ToEqual(ns2.scopes[i].AsString().data());
			++i;
		}
	});

	It("Can find id from namespace", []()
	{
		ast::Tree ast;
		ast::Id parent = p::AddId(ast);
		ast.Add<ast::CModule>(parent);
		ast.Add(parent, ast::CNamespace{"A"});

		ast::Id classId = ast::CreateType(ast, ASTModule::classType, "B");
		p::AttachId(ast, parent, classId);

		ast::Id class2Id = ast::CreateType(ast, ASTModule::classType, "B2");
		p::AttachId(ast, parent, class2Id);

		ast::Id functionId  = ast::AddFunction({ast, classId}, "C");
		ast::Id function2Id = ast::AddFunction({ast, classId}, "C2");


		Expect(ast::FindIdFromNamespace(ast, {"A"})).ToEqual(parent);
		Expect(ast::FindIdFromNamespace(ast, {"A", "B"})).ToEqual(classId);
		Expect(ast::FindIdFromNamespace(ast, {"A", "B2"})).ToEqual(class2Id);
		Expect(ast::FindIdFromNamespace(ast, {"A", "B", "C"})).ToEqual(functionId);
		Expect(ast::FindIdFromNamespace(ast, {"A", "B", "C2"})).ToEqual(function2Id);

		Expect(ast::FindIdFromNamespace(ast, {"N"})).ToEqual(ast::NoId);
		Expect(ast::FindIdFromNamespace(ast, {"A", "N"})).ToEqual(ast::NoId);
	});
});
