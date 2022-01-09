// Copyright 2015-2021 Piperift - All rights reserved

#define RIFT_ENABLE_PROFILER 0

#include <AST/Access.h>
#include <AST/Tree.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


struct Type
{
	bool value = false;
};
struct TypeTwo
{};


go_bandit([]() {
	describe("AST", []() {
		it("Can copy tree", [&]() {
			AST::Tree origin;
			AST::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<Type>().Bind([&calledAdd, &origin](AST::Tree& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<Type>(id);
			AssertThat(calledAdd, Equals(true));

			AST::Tree target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<Type>(id), Equals(true));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<Type>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeTwo>().Bind([&calledAdd, &target](AST::Tree& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeTwo>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			AST::Tree origin;
			AST::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<Type>().Bind([&calledAdd, &origin](AST::Tree& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<Type>(id);
			AssertThat(calledAdd, Equals(true));

			AST::Tree target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<Type>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeTwo>().Bind([&calledAdd, &target](AST::Tree& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeTwo>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can assure pool", [&]() {
			AST::Tree origin;
			AST::TPool<Type>& pool = origin.AssurePool<Type>();
			AssertThat(&origin, Equals(&pool.GetAST()));
		});
	});
});
