// Copyright 2015-2023 Piperift - All rights reserved

#include <AST/Components/CNamespace.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/TypeUtils.h>
#include <ASTModule.h>
#include <bandit/assertion_frameworks/snowhouse/assert.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace rift;


go_bandit([]() {
	describe("AST.Namespaces", []() {
		it("Can get namespaces", [&]() {
			ast::Tree ast;

			ast::Id functionId = ast::AddFunction({ast, ast::NoId}, "TestFunction");
			AssertThat(
			    ast::GetNamespace(ast, functionId).ToString().c_str(), Equals("@TestFunction"));
			AssertThat(ast::GetParentNamespace(ast, functionId).ToString().c_str(), Equals(""));

			ast::Id classBId    = ast::CreateType(ast, ASTModule::classType, "TestClass");
			ast::Id functionBId = ast::AddFunction({ast, classBId}, "TestFunction");
			AssertThat(ast::GetNamespace(ast, functionBId).ToString().c_str(),
			    Equals("@TestClass.TestFunction"));
			AssertThat(
			    ast::GetParentNamespace(ast, functionBId).ToString().c_str(), Equals("@TestClass"));

			ast::Id parentC = ast.Create();
			ast.Add<ast::CModule>(parentC);
			ast.Add(parentC, ast::CNamespace{"SomeScope"});
			ast::Id classCId = ast::CreateType(ast, ASTModule::classType, "TestClass");
			p::AttachId(ast, parentC, classCId);
			ast::Id functionCId = ast::AddFunction({ast, classCId}, "TestFunction");
			AssertThat(ast::GetNamespace(ast, functionCId).ToString().c_str(),
			    Equals("@SomeScope.TestClass.TestFunction"));
			AssertThat(ast::GetParentNamespace(ast, functionCId).ToString().c_str(),
			    Equals("@SomeScope.TestClass"));
		});

		it("Can get local namespaces", [&]() {
			ast::Tree ast;

			ast::Id parent = ast.Create();
			ast.Add<ast::CModule>(parent);
			ast.Add(parent, ast::CNamespace{"SomeModule"});
			ast::Id classId = ast::CreateType(ast, ASTModule::classType, "TestClass");
			p::AttachId(ast, parent, classId);
			ast::Id functionId = ast::AddFunction({ast, classId}, "TestFunction");
			p::String ns       = ast::GetNamespace(ast, functionId).ToString(true);
			AssertThat(ns.c_str(), Equals("TestClass.TestFunction"));
		});

		it("Can initialize", [&]() {
			ast::Namespace ns0{};
			AssertThat(ns0.scopes[0].IsNone(), Equals(true));
			AssertThat(ns0.Size(), Equals(0));
			AssertThat(ns0.IsEmpty(), Equals(true));

			ast::Namespace ns1{"A"};
			AssertThat(ns1.scopes[0].AsString().data(), Equals("A"));
			AssertThat(ns1.scopes[1].IsNone(), Equals(true));
			AssertThat(ns1.Size(), Equals(1));
			AssertThat(ns1.IsEmpty(), Equals(false));

			ast::Namespace ns2{"A", "B"};
			AssertThat(ns2.scopes[0].AsString().data(), Equals("A"));
			AssertThat(ns2.scopes[1].AsString().data(), Equals("B"));
			AssertThat(ns2.scopes[2].IsNone(), Equals(true));
			AssertThat(ns2.Size(), Equals(2));
			AssertThat(ns2.IsEmpty(), Equals(false));
		});

		it("Can iterate", [&]() {
			ast::Namespace ns0{};
			for (const Tag& name : ns0)
			{
				Assert();
			}

			ast::Namespace ns1{"C"};
			for (const Tag& name : ns1)
			{
				AssertThat(name.AsString().data(), Equals("C"));
			}

			ast::Namespace ns2{"A", "B"};
			i32 i = 0;
			for (const Tag& name : ns2)
			{
				AssertThat(name.AsString().data(), Equals(ns2.scopes[i].AsString().data()));
				++i;
			}
		});

		it("Can find id from namespace", [&]() {
			ast::Tree ast;
			ast::Id parent = ast.Create();
			ast.Add<ast::CModule>(parent);
			ast.Add(parent, ast::CNamespace{"A"});

			ast::Id classId = ast::CreateType(ast, ASTModule::classType, "B");
			p::AttachId(ast, parent, classId);

			ast::Id class2Id = ast::CreateType(ast, ASTModule::classType, "B2");
			p::AttachId(ast, parent, class2Id);

			ast::Id functionId  = ast::AddFunction({ast, classId}, "C");
			ast::Id function2Id = ast::AddFunction({ast, classId}, "C2");


			AssertThat(ast::FindIdFromNamespace(ast, {"A"}), Equals(parent));
			AssertThat(ast::FindIdFromNamespace(ast, {"A", "B"}), Equals(classId));
			AssertThat(ast::FindIdFromNamespace(ast, {"A", "B2"}), Equals(class2Id));
			AssertThat(ast::FindIdFromNamespace(ast, {"A", "B", "C"}), Equals(functionId));
			AssertThat(ast::FindIdFromNamespace(ast, {"A", "B", "C2"}), Equals(function2Id));

			AssertThat(ast::FindIdFromNamespace(ast, {"N"}), Equals(ast::NoId));
			AssertThat(ast::FindIdFromNamespace(ast, {"A", "N"}), Equals(ast::NoId));
		});
	});
});
