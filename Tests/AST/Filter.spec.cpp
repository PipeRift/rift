// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"

#include <AST/Filter.h>
#include <AST/Tree.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::AST;

namespace snowhouse
{
	template<>
	struct Stringizer<AST::Id>
	{
		static std::string ToString(AST::Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<AST::Id>(id) << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


struct Type
{};
struct TypeTwo
{};
struct TypeThree
{};


go_bandit([]() {
	AST::Tree ast;
	AST::Id id1;
	AST::Id id2;
	AST::Id id3;
	describe("AST.Filter", [&]() {
		before_each([&]() {
			ast = {};
			id1 = ast.Create();
			id2 = ast.Create();
			id3 = ast.Create();
			ast.Add<Type>(id1);
			ast.Add<Type, TypeTwo, TypeThree>(id2);
			ast.Add<TypeTwo, TypeThree>(id3);
		});

		describe("Iteration", [&]() {
			it("Can iterate with Each", [&]() {
				auto filter = ast.Filter<Type>();

				TArray<AST::Id> iteratedIds;
				filter.Each([&iteratedIds](AST::Id id) {
					iteratedIds.Add(id);
				});
				AssertThat(iteratedIds.Size(), Equals(2));
				AssertThat(iteratedIds[0], Equals(id2));
				AssertThat(iteratedIds[1], Equals(id1));
			});

			it("Can iterate with EachBreak", [&]() {
				auto filter = ast.Filter<Type>();

				TArray<AST::Id> iteratedIds;
				filter.EachBreak([&iteratedIds](AST::Id id) {
					iteratedIds.Add(id);
					return false;
				});
				AssertThat(iteratedIds.Size(), Equals(1));
				AssertThat(iteratedIds[0], Equals(id2));
			});

			it("Can iterate with begin/end", [&]() {
				auto filter = ast.Filter<Type>();

				TArray<AST::Id> iteratedIds;
				for (AST::Id id : filter)
				{
					iteratedIds.Add(id);
				}
				AssertThat(iteratedIds.Size(), Equals(2));
				AssertThat(iteratedIds[0], Equals(id2));
				AssertThat(iteratedIds[1], Equals(id1));
			});
		});

		it("Can include multiple components", [&]() {
			auto filter = ast.Filter<TypeTwo, TypeThree>();

			TArray<AST::Id> iteratedIds;
			for (AST::Id id : filter)
			{
				iteratedIds.Add(id);
			}
			AssertThat(iteratedIds.Size(), Equals(2));
			AssertThat(iteratedIds[0], Equals(id3));
			AssertThat(iteratedIds[1], Equals(id2));
		});

		it("Can exclude components", [&]() {
			auto filter = ast.Filter<TypeTwo, TypeThree>(TExclude<Type>{});


			AssertThat(filter.Has(id1), Equals(false));
			AssertThat(filter.Has(id2), Equals(false));
			AssertThat(filter.Has(id3), Equals(true));

			TArray<AST::Id> iteratedIds;
			for (AST::Id id : filter)
			{
				iteratedIds.Add(id);
			}
			AssertThat(iteratedIds.Size(), Equals(1));
			AssertThat(iteratedIds[0], Equals(id3));
		});
	});
});
