// Copyright 2015-2021 Piperift - All rights reserved

#define RIFT_ENABLE_PROFILER 0

#include <AST/Access.h>
#include <AST/Tree.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


struct Type
{};
struct TypeTwo
{};
struct TypeThree
{};


go_bandit([]() {
	describe("AST.Access", []() {
		describe("Templated", []() {
			it("Can cache pools", [&]() {
				AST::Tree tree;
				auto access = tree.Access<Type>(AST::TExclude<TypeTwo>{});

				AssertThat(access.GetPool<Type>(), Equals(tree.FindPool<Type>()));
				AssertThat(access.GetPool<TypeTwo>(), Equals(nullptr));
				AssertThat(access.GetExcludedPool<Type>(), Equals(nullptr));
				AssertThat(access.GetExcludedPool<TypeTwo>(), Equals(tree.FindPool<TypeTwo>()));
			});

			it("Can check if contained", [&]() {
				AST::Tree tree;
				AST::TPool<Type>& pool = tree.AssurePool<Type>();
				auto access            = tree.Access<Type>();
				auto accessExcluded    = tree.Access<Type>(AST::TExclude<TypeTwo>{});
				AST::Id id             = AST::NoId;
				AssertThat(access.Has(id), Is().False());
				AssertThat(accessExcluded.Has(id), Is().False());

				id = tree.Create();
				AssertThat(access.Has(id), Is().False());
				AssertThat(accessExcluded.Has(id), Is().False());

				tree.Add<Type>(id);
				AssertThat(access.Has(id), Is().True());
				AssertThat(accessExcluded.Has(id), Is().True());

				tree.Add<TypeTwo>(id);
				AssertThat(access.Has(id), Is().True());
				AssertThat(accessExcluded.Has(id), Is().False());
			});

			it("Can initialize superset", [&]() {
				AST::Tree tree;
				AST::TPool<Type>& typePool = tree.AssurePool<Type>();

				AST::Access::In<Type, TypeTwo> access = tree.Access<Type, TypeTwo>();
				AST::Access::In<Type> superset{access};

				AssertThat(access.GetPool<Type>(), Equals(&typePool));
				AssertThat(superset.GetPool<Type>(), Equals(&typePool));
			});
		});

		xdescribe("Runtime", []() {
			xit("Can cache pools", [&]() {});
		});
	});
});
