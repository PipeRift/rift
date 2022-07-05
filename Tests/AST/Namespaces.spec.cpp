// Copyright 2015-2022 Piperift - All rights reserved

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
			p::String ns;
			AST::GetNamespace(ast, functionId, ns);
			AssertThat(ns.c_str(), Equals(""));

			AST::Id classBId    = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Id functionBId = Types::AddFunction({ast, classBId}, "TestFunction");
			p::String nsB;
			AST::GetNamespace(ast, functionBId, nsB);
			AssertThat(nsB.c_str(), Equals("TestClass"));

			AST::Id parentC = ast.Create();
			ast.Add<CModule>(parentC);
			ast.Add(parentC, CNamespace{"SomeScope"});
			AST::Id classCId = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Hierarchy::AddChildren(ast, parentC, classCId);
			AST::Id functionCId = Types::AddFunction({ast, classCId}, "TestFunction");
			p::String nsC;
			AST::GetNamespace(ast, functionCId, nsC);
			AssertThat(nsC.c_str(), Equals("SomeScope.TestClass"));
		});

		it("Can get local namespaces", [&]() {
			AST::Tree ast;

			AST::Id parent = ast.Create();
			ast.Add<CModule>(parent);
			ast.Add(parent, CNamespace{"SomeModule"});
			AST::Id classId = Types::CreateType(ast, RiftType::Class, "TestClass");
			AST::Hierarchy::AddChildren(ast, parent, classId);
			AST::Id functionId = Types::AddFunction({ast, classId}, "TestFunction");
			p::String ns;
			AST::GetLocalNamespace(ast, functionId, ns);
			AssertThat(ns.c_str(), Equals("TestClass"));
		});
	});
});
