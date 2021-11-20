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
	float a = 0.f;
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

		xit("Can add many components", [&]() {});
		xit("Can remove many components", [&]() {});

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
	});
});
