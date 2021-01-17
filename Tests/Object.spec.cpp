// Copyright 2015-2020 Piperift - All rights reserved

#include <CoreObject.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


class TestObject : public Object
{
	CLASS(TestObject, Object);

public:
	bool bConstructed = false;

	virtual void Construct() override
	{
		Super::Construct();
		bConstructed = true;
	}
};


go_bandit([]() {
	describe("Object", []() {
		describe("Pointers", []() {
			it("Can create object", [&]() {
				auto owner = Create<TestObject>();

				AssertThat(owner.Get(), Is().Not().EqualTo(nullptr));
				AssertThat(owner->bConstructed, Equals(true));
			});

			it("Can create object with owner", [&]() {
				auto owner = Create<TestObject>();
				auto owner2 = Create<TestObject>(owner);

				AssertThat(owner2->bConstructed, Equals(true));
				AssertThat(owner2->GetOwner().IsValid(), Equals(true));
			});
		});
	});
});
