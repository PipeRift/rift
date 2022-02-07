// Copyright 2015-2022 Piperift - All rights reserved

#define RIFT_ENABLE_PROFILER 0

#include <AST/Access.h>
#include <AST/FilterAccess.h>
#include <AST/Tree.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::AST;


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
				Tree tree;
				TAccess<Type, const TypeTwo> access{tree};

				AssertThat(access.GetPool<Type>(), Equals(tree.GetPool<Type>()));
				AssertThat(access.GetPool<const Type>(), Equals(tree.GetPool<Type>()));
				AssertThat(access.GetPool<const TypeTwo>(), Equals(tree.GetPool<TypeTwo>()));
			});

			it("Can check if contained", [&]() {
				Tree tree;
				TPool<Type>& pool = tree.AssurePool<Type>();
				TAccess<Type> access{tree};
				TAccess<const Type> accessConst{tree};
				Id id = NoId;
				AssertThat(access.Has(id), Is().False());
				AssertThat(accessConst.Has(id), Is().False());

				id = tree.Create();
				AssertThat(access.Has(id), Is().False());
				AssertThat(accessConst.Has(id), Is().False());

				tree.Add<Type>(id);
				AssertThat(access.Has(id), Is().True());
				AssertThat(accessConst.Has(id), Is().True());

				TAccess<Type, TypeTwo> access2{tree};
				tree.Add<TypeTwo>(id);
				AssertThat(access.Has(id), Is().True());
				AssertThat(access2.Has(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				Tree tree;
				TPool<Type>& typePool = tree.AssurePool<Type>();

				TAccess<Type, TypeTwo> access1{tree};
				TAccess<Type> superset1{access1};
				AssertThat(superset1.GetPool<Type>(), Equals(&typePool));

				TAccess<Type, TypeTwo> access2{tree};
				TAccess<const Type> superset2{access2};
				AssertThat(superset2.GetPool<const Type>(), Equals(&typePool));

				TAccess<const Type, TypeTwo> access3{tree};
				TAccess<const Type> superset3{access3};
				AssertThat(superset1.GetPool<Type>(), Equals(&typePool));
			});
		});

		xdescribe("Runtime", []() {
			xit("Can cache pools", [&]() {});
		});
	});
});
