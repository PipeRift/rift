// Copyright 2015-2022 Piperift - All rights reserved

#include <AST/IdRegistry.h>
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


go_bandit([]() {
	describe("AST.IdRegistry", []() {
		it("Can create one id", [&]() {
			AST::IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));

			AST::Id id = ids.Create();
			AssertThat(id, !Equals(AST::Id(AST::NoId)));
			AssertThat(ids.IsValid(id), Is().True());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can remove one id", [&]() {
			AST::IdRegistry ids;
			AST::Id id = ids.Create();
			AssertThat(ids.Size(), Equals(1));

			AssertThat(ids.Destroy(id), Is().True());
			AssertThat(ids.IsValid(id), Is().False());
			AssertThat(ids.Size(), Equals(0));
		});

		it("Can create two and remove first", [&]() {
			AST::IdRegistry ids;

			AST::Id id1 = ids.Create();
			AST::Id id2 = ids.Create();
			AssertThat(ids.Destroy(id1), Is().True());
			AssertThat(ids.IsValid(id1), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create two and remove last", [&]() {
			AST::IdRegistry ids;

			AST::Id id1 = ids.Create();
			AST::Id id2 = ids.Create();

			AssertThat(ids.Destroy(id2), Is().True());
			AssertThat(ids.IsValid(id2), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create many ids", [&]() {
			AST::IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));

			TArray<AST::Id> list(3);
			ids.Create(list);

			AssertThat(ids.Size(), Equals(3));
			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(list[i], Equals(AST::Id(i)));
				AssertThat(ids.IsValid(list[i]), Is().True());
			}
		});

		it("Can remove many ids", [&]() {
			AST::IdRegistry ids;
			TArray<AST::Id> list(3);
			ids.Create(list);
			AssertThat(ids.Size(), Equals(3));

			AssertThat(ids.Destroy(list), Is().True());
			AssertThat(ids.Size(), Equals(0));

			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(ids.IsValid(list[i]), Is().False());
			}
		});
	});
});
