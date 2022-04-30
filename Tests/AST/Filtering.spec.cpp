// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"

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
	AST::Id id4;
	AST::Id id5;
	describe("AST.Filtering", [&]() {
		before_each([&]() {
			ast = {};
			id1 = ast.Create();
			id2 = ast.Create();
			id3 = ast.Create();
			id4 = ast.Create();
			id5 = ast.Create();
			ast.Add<Type>(id1);
			ast.Add<Type, TypeTwo, TypeThree>(id2);
			ast.Add<TypeTwo, TypeThree>(id3);
			ast.Add<TypeTwo, TypeThree>(id4);
			ast.Add<TypeTwo>(id5);
		});

		describe("ListAny/ListAll", [&]() {
			it("Can get list matching all", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ast};
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
				TAccess<Type, TypeTwo, TypeThree> access{ast};
				TArray<AST::Id> typeIds = AST::ListAny<Type>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<AST::Id> type2Ids = AST::ListAny<Type, TypeThree>(access);
				AssertThat(type2Ids.Contains(id1), Is().True());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Doesn't list removed ids", [&]() {
				TAccess<TypeTwo> access{ast};
				ast.Destroy(id2);    // Remove first in the pool
				ast.Destroy(id3);    // Remove last in the pool
				ast.Destroy(id4);    // Remove last in the pool

				TArray<AST::Id> ids = AST::ListAll<TypeTwo>(access);
				AssertThat(ids.Contains(AST::NoId), Is().False());
				AssertThat(ids.Size(), Equals(1));
			});
		});

		describe("RemoveIf", [&]() {
			it("Removes ids containing component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ast};
				TArray<AST::Id> typeIds = AST::ListAny<Type>(access);

				AST::RemoveIf<TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids not containing component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ast};
				TArray<AST::Id> typeIds = AST::ListAny<Type>(access);

				AST::RemoveIfNot<TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().False());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids containing multiple component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ast};
				TArray<AST::Id> typeIds = AST::ListAny<Type, TypeTwo, TypeThree>(access);

				AST::RemoveIf<TypeTwo, TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});
		});

		describe("GetIf", [&]() {
			it("Finds ids containing a component from a list", [&]() {
				TArray<AST::Id> source{id1, id2, id3};

				TAccess<Type> access{ast};
				TArray<AST::Id> typeIds = AST::GetIf<Type>(access, source);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Finds ids not containing a component from a list", [&]() {
				TArray<AST::Id> source{id1, id2, id3};

				TAccess<Type> access{ast};
				TArray<AST::Id> ids = AST::GetIfNot<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
			});
		});

		describe("ExtractIf", [&]() {
			it("Finds and removes ids containing a component from a list", [&]() {
				TArray<AST::Id> source{id1, id2, id3};

				TAccess<Type> access{ast};
				TArray<AST::Id> ids = AST::ExtractIf<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().True());
				AssertThat(ids.Contains(id2), Is().True());
				AssertThat(ids.Contains(id3), Is().False());
				AssertThat(source.Contains(id1), Is().False());
				AssertThat(source.Contains(id2), Is().False());
				AssertThat(source.Contains(id3), Is().True());
			});

			it("Finds and removes ids not containing a component from a list", [&]() {
				TArray<AST::Id> source{id1, id2, id3};

				TAccess<Type> access{ast};
				TArray<AST::Id> ids = AST::ExtractIfNot<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
				AssertThat(source.Contains(id1), Is().True());
				AssertThat(source.Contains(id2), Is().True());
				AssertThat(source.Contains(id3), Is().False());
			});
		});

		it("Can filter directly from AST", [&]() {
			TArray<AST::Id> ids1 = AST::ListAll<Type>(ast);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<AST::Id> ids2 = AST::ListAny<Type>(ast);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<AST::Id> ids3 = AST::ListAny<Type>(ast);
			AST::RemoveIf<TypeThree>(ast, ids3);
			AssertThat(ids3.Contains(id1), Is().True());

			TArray<AST::Id> ids4 = AST::ListAny<Type>(ast);
			AST::RemoveIfNot<TypeThree>(ast, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
