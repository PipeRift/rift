// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/assertion_frameworks/snowhouse/assert.h"

#include <AST/Components/CNamespace.h>
#include <AST/Tree.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/TypeUtils.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace rift;


go_bandit([]() {
	describe("AST.Namespaces", []() {
		it("Can get namespaces", [&]() {
			AST::Tree ast;

			AST::Id functionId = Types::AddFunction({ast, AST::NoId}, "TestFunction");
			AssertThat(
			    AST::GetNamespace(ast, functionId).ToString().c_str(), Equals("@TestFunction"));
			AssertThat(AST::GetParentNamespace(ast, functionId).ToString().c_str(), Equals(""));

			AST::Id classBId    = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Id functionBId = Types::AddFunction({ast, classBId}, "TestFunction");
			AssertThat(AST::GetNamespace(ast, functionBId).ToString().c_str(),
			    Equals("@TestClass.TestFunction"));
			AssertThat(
			    AST::GetParentNamespace(ast, functionBId).ToString().c_str(), Equals("@TestClass"));

			AST::Id parentC = ast.Create();
			ast.Add<CModule>(parentC);
			ast.Add(parentC, CNamespace{"SomeScope"});
			AST::Id classCId = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Hierarchy::AddChildren(ast, parentC, classCId);
			AST::Id functionCId = Types::AddFunction({ast, classCId}, "TestFunction");
			AssertThat(AST::GetNamespace(ast, functionCId).ToString().c_str(),
			    Equals("@SomeScope.TestClass.TestFunction"));
			AssertThat(AST::GetParentNamespace(ast, functionCId).ToString().c_str(),
			    Equals("@SomeScope.TestClass"));
		});

		it("Can get local namespaces", [&]() {
			AST::Tree ast;

			AST::Id parent = ast.Create();
			ast.Add<CModule>(parent);
			ast.Add(parent, CNamespace{"SomeModule"});
			AST::Id classId = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Hierarchy::AddChildren(ast, parent, classId);
			AST::Id functionId = Types::AddFunction({ast, classId}, "TestFunction");
			p::String ns = AST::GetNamespace(ast, functionId).ToString(AST::LocalNamespace::Yes);
			AssertThat(ns.c_str(), Equals("TestClass.TestFunction"));
		});

		it("Can initialize", [&]() {
			AST::Namespace ns0{};
			AssertThat(ns0.scopes[0].IsNone(), Equals(true));
			AssertThat(ns0.Size(), Equals(0));
			AssertThat(ns0.IsEmpty(), Equals(true));

			AST::Namespace ns1{"A"};
			AssertThat(ns1.scopes[0].ToString().c_str(), Equals("A"));
			AssertThat(ns1.scopes[1].IsNone(), Equals(true));
			AssertThat(ns1.Size(), Equals(1));
			AssertThat(ns1.IsEmpty(), Equals(false));

			AST::Namespace ns2{"A", "B"};
			AssertThat(ns2.scopes[0].ToString().c_str(), Equals("A"));
			AssertThat(ns2.scopes[1].ToString().c_str(), Equals("B"));
			AssertThat(ns2.scopes[2].IsNone(), Equals(true));
			AssertThat(ns2.Size(), Equals(2));
			AssertThat(ns2.IsEmpty(), Equals(false));
		});

		it("Can iterate", [&]() {
			AST::Namespace ns0{};
			for (const Name& name : ns0)
			{
				Assert();
			}

			AST::Namespace ns1{"C"};
			for (const Name& name : ns1)
			{
				AssertThat(name.ToString().c_str(), Equals("C"));
			}

			AST::Namespace ns2{"A", "B"};
			i32 i = 0;
			for (const Name& name : ns2)
			{
				AssertThat(name.ToString().c_str(), Equals(ns2.scopes[i].ToString().c_str()));
				++i;
			}
		});

		it("Can find id from namespace", [&]() {
			AST::Tree ast;
			AST::Id parent = ast.Create();
			ast.Add<CModule>(parent);
			ast.Add(parent, CNamespace{"A"});

			AST::Id classId = Types::CreateType(ast, RiftType::Class, "B");
			AST::Hierarchy::AddChildren(ast, parent, classId);

			AST::Id class2Id = Types::CreateType(ast, RiftType::Class, "B2");
			AST::Hierarchy::AddChildren(ast, parent, class2Id);

			AST::Id functionId  = Types::AddFunction({ast, classId}, "C");
			AST::Id function2Id = Types::AddFunction({ast, classId}, "C2");


			AssertThat(AST::FindIdFromNamespace(ast, {"A"}), Equals(parent));
			AssertThat(AST::FindIdFromNamespace(ast, {"A", "B"}), Equals(classId));
			AssertThat(AST::FindIdFromNamespace(ast, {"A", "B2"}), Equals(class2Id));
			AssertThat(AST::FindIdFromNamespace(ast, {"A", "B", "C"}), Equals(functionId));
			AssertThat(AST::FindIdFromNamespace(ast, {"A", "B", "C2"}), Equals(function2Id));

			AssertThat(AST::FindIdFromNamespace(ast, {"N"}), Equals(AST::NoId));
			AssertThat(AST::FindIdFromNamespace(ast, {"A", "N"}), Equals(AST::NoId));
		});
	});
});
