// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"
#define RIFT_ENABLE_PROFILER 0

#include <AST/Filtering.h>
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
	describe("AST.Filtering", [&]() {
		before_each([&]() {
			ast = {};
			id1 = ast.Create();
			id2 = ast.Create();
			id3 = ast.Create();
			ast.Add<Type>(id1);
			ast.Add<Type, TypeTwo, TypeThree>(id2);
			ast.Add<TypeTwo, TypeThree>(id3);
		});

		it("Can get list matching all", [&]() {
			AST::TAccess<const Type, const TypeTwo, const TypeThree> access{ast};
			TArray<AST::Id> typeIds = AST::ListAll<Type>(access);
			AssertThat(typeIds.Contains(id1), Is().True());
			AssertThat(typeIds.Contains(id2), Is().True());
			AssertThat(typeIds.Contains(id3), Is().False());

			TArray<AST::Id> type2Ids = AST::ListAll<TypeTwo, TypeThree>(access);
			AssertThat(type2Ids.Contains(id1), Is().False());
			AssertThat(type2Ids.Contains(id2), Is().True());
			AssertThat(type2Ids.Contains(id3), Is().True());
		});

		it("Can get list matching any", [&]() {
			AST::TAccess<const Type, const TypeTwo, const TypeThree> access{ast};
			TArray<AST::Id> typeIds = AST::ListAny<Type>(access);
			AssertThat(typeIds.Contains(id1), Is().True());
			AssertThat(typeIds.Contains(id2), Is().True());
			AssertThat(typeIds.Contains(id3), Is().False());

			TArray<AST::Id> type2Ids = AST::ListAny<Type, TypeThree>(access);
			AssertThat(type2Ids.Contains(id1), Is().True());
			AssertThat(type2Ids.Contains(id2), Is().True());
			AssertThat(type2Ids.Contains(id3), Is().True());
		});

		it("Removes ids containing component", [&]() {
			AST::TAccess<const Type, const TypeTwo, const TypeThree> access{ast};
			TArray<AST::Id> typeIds = AST::ListAny<Type>(access);

			AST::RemoveIf<TypeThree>(access, typeIds);
			AssertThat(typeIds.Contains(id1), Is().True());
			AssertThat(typeIds.Contains(id2), Is().False());
			AssertThat(typeIds.Contains(id3), Is().False());
		});

		it("Removes ids not containing component", [&]() {
			AST::TAccess<const Type, const TypeTwo, const TypeThree> access{ast};
			TArray<AST::Id> typeIds = AST::ListAny<Type>(access);

			AST::RemoveIfNot<TypeThree>(access, typeIds);
			AssertThat(typeIds.Contains(id1), Is().False());
			AssertThat(typeIds.Contains(id2), Is().True());
			AssertThat(typeIds.Contains(id3), Is().False());
		});

		it("Can filter directly from AST", [&]() {
			TArray<AST::Id> ids1 = AST::ListAll<Type>(ast);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<AST::Id> ids2 = AST::ListAny<Type>(ast);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<AST::Id> ids4 = AST::ListAny<Type>(ast);
			AST::RemoveIfNot<TypeThree>(ast, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
