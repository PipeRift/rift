// Copyright 2015-2021 Piperift - All rights reserved

#define RIFT_ENABLE_PROFILER 0

#include <AST/Access.h>
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
	describe("AST.PoolAccess", []() {
		describe("Templated", []() {
			it("Can be empty", [&]() {
				// using EmptyAccess = AST::TPoolAccess<>;

				// EmptyAccess access{};
				// AssertThat(access.CanRead<Type>(), Equals(false));
			});
		});

		describe("Not Templated", []() {
			it("Can be empty", [&]() {
				// AST::PoolAccess access{};
			});
		});
	});
});
