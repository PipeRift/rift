// Copyright 2015-2021 Piperift - All rights reserved

#define RIFT_ENABLE_PROFILER 0

#include <AST/Tree.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace std::chrono_literals;

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


struct EmptyComponent
{};

struct NonEmptyComponent
{
	i32 a = 0;
};


go_bandit([]() {
	describe("AST.Components", []() {
		it("Can add one component", [&]() {
			AST::Tree ast;
			AST::Id id = ast.Create();
			AssertThat(ast.Has<EmptyComponent>(id), Is().False());
			AssertThat(ast.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ast.TryGet<NonEmptyComponent>(id), Equals(nullptr));

			ast.Add<EmptyComponent>(id);
			AssertThat(ast.Has<EmptyComponent>(id), Is().True());
			AssertThat(ast.TryGet<EmptyComponent>(id), Equals(nullptr));

			ast.Add<NonEmptyComponent>(id);
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ast.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
		});

		it("Can remove one component", [&]() {
			AST::Tree ast;
			AST::Id id = ast.Create();
			ast.Add<EmptyComponent, NonEmptyComponent>(id);

			ast.Remove<EmptyComponent>(id);
			AssertThat(ast.Has<EmptyComponent>(id), Is().False());
			AssertThat(ast.TryGet<EmptyComponent>(id), Equals(nullptr));

			ast.Remove<NonEmptyComponent>(id);
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ast.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Can add many components", [&]() {
			AST::Tree ast;
			TArray<AST::Id> ids{3};
			ast.Create(ids);
			ast.Add<NonEmptyComponent>(ids, {2});

			for (AST::Id id : ids)
			{
				auto* data = ast.TryGet<NonEmptyComponent>(id);
				AssertThat(data, !Equals(nullptr));
				AssertThat(data->a, Equals(2));
			}
		});
		it("Can remove many components", [&]() {
			AST::Tree ast;
			TArray<AST::Id> ids{3};
			ast.Create(ids);
			ast.Add<NonEmptyComponent>(ids, {2});

			TSpan<AST::Id> firstTwo{ids.Data(), ids.Data() + 2};
			ast.Remove<NonEmptyComponent>(firstTwo);

			AssertThat(ast.TryGet<NonEmptyComponent>(ids[0]), Equals(nullptr));
			AssertThat(ast.TryGet<NonEmptyComponent>(ids[1]), Equals(nullptr));
			AssertThat(ast.TryGet<NonEmptyComponent>(ids[2]), !Equals(nullptr));
		});

		it("Components are removed after node is deleted", [&]() {
			AST::Tree ast;
			AST::Id id = ast.Create();
			ast.Add<EmptyComponent, NonEmptyComponent>(id);

			ast.Destroy(id);
			AssertThat(ast.IsValid(id), Is().False());

			AssertThat(ast.Has<EmptyComponent>(id), Is().False());
			AssertThat(ast.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ast.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Components keep state when added", [&]() {
			AST::Tree ast;
			AST::Id id = ast.Create();
			ast.Add<NonEmptyComponent>(id, {2});
			AssertThat(ast.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
			AssertThat(ast.Get<NonEmptyComponent>(id).a, Equals(2));
		});

		it("Can copy registry", []() {
			AST::Tree asta;

			AST::Id id = asta.Create();
			asta.Add<EmptyComponent, NonEmptyComponent>(id);
			AST::Id id2 = asta.Create();
			asta.Add<NonEmptyComponent>(id2, {2});

			AST::Tree astb{asta};
			AssertThat(astb.Has<EmptyComponent>(id), Is().True());
			AssertThat(astb.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(astb.TryGet<NonEmptyComponent>(id), !Equals(nullptr));

			// Holds component values
			AssertThat(astb.Get<NonEmptyComponent>(id2).a, Equals(2));
		});

		it("Can check components", [&]() {
			AST::Tree ast;
			AST::Id id = AST::NoId;
			AssertThat(ast.Has<EmptyComponent>(id), Is().False());
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().False());

			id = ast.Create();
			AssertThat(ast.Has<EmptyComponent>(id), Is().False());
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().False());

			ast.Add<EmptyComponent, NonEmptyComponent>(id);
			AssertThat(ast.Has<EmptyComponent>(id), Is().True());
			AssertThat(ast.Has<NonEmptyComponent>(id), Is().True());
		});
	});
});
