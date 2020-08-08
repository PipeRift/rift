// Copyright 2015-2020 Piperift - All rights reserved

#include <Pointers/PtrOwner.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace VCLang;

struct EmptyStruct
{};

struct MockStruct
{
	bool bCalledNew = false;
	static bool bCalledDelete;
};

template <typename T>
struct TestPtrBuilder : VCLang::PtrBuilder<T>
{
	template <typename... Args>
	static T* New(Args&&... args)
	{
		T* ptr = new T(std::forward<Args>(args)...);
		ptr->bCalledNew = true;
		return ptr;
	}

	static void Delete(void* ptr)
	{
		T::bCalledDelete = true;
		delete static_cast<T*>(ptr);
	}
};


go_bandit([]() {
	describe("PtrOwner", []() {
		describe("Owner pointer", []() {
			it("Can initialize to empty", [&]() {
				PtrOwner<EmptyStruct> ptr;
				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.Get(), Equals(nullptr));
			});

			it("Can instantiate", [&]() {
				PtrOwner<EmptyStruct> ptr = MakeOwned<EmptyStruct>();
				AssertThat(ptr.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Is().Not().EqualTo(nullptr));
			});

			it("Owner can release", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				AssertThat(owner.IsValid(), Equals(true));

				owner.Release();
				AssertThat(owner.IsValid(), Equals(false));
			});

			it("Owner is released when destroyed", [&]() {
				Ptr<EmptyStruct> ptr;
				{
					PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
					ptr = owner;
					AssertThat(ptr.IsValid(), Equals(true));
				}
				AssertThat(ptr.IsValid(), Equals(false));
			});

			describe("Ptr Builder", []() {
				it("Calls custom new", [&]() {
					auto owner = MakeOwned<MockStruct, TestPtrBuilder>();
					AssertThat(owner->bCalledNew, Equals(true));
				});

				it("Calls custom delete", [&]() {
					MockStruct::bCalledDelete = false;
					auto owner = MakeOwned<MockStruct, TestPtrBuilder>();
					AssertThat(MockStruct::bCalledDelete, Equals(false));
					owner.Release();
					AssertThat(MockStruct::bCalledDelete, Equals(true));
				});
			});
		});

		describe("Weak pointer", []() {
			it("Can initialize to empty", [&]() {
				Ptr<EmptyStruct> ptr;
				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.Get(), Equals(nullptr));
			});

			it("Can initialize from owner", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				Ptr<EmptyStruct> ptr = owner;

				AssertThat(ptr.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Is().Not().EqualTo(nullptr));
			});

			it("Can copy from other weak", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				auto* raw = *owner;
				Ptr<EmptyStruct> ptr = owner;
				Ptr<EmptyStruct> ptr2 = ptr;

				AssertThat(ptr2.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Equals(raw));
				AssertThat(ptr2.Get(), Equals(raw));
			});

			it("Can move from other weak", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				auto* raw = *owner;
				Ptr<EmptyStruct> ptr = owner;
				Ptr<EmptyStruct> ptr2 = MoveTemp(ptr);

				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr2.IsValid(), Equals(true));

				AssertThat(ptr.Get(), Equals(nullptr));
				AssertThat(ptr2.Get(), Equals(raw));
			});

			it("Ptr is null after IsValid() == false", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				Ptr<EmptyStruct> ptr = owner;
				owner.Release();

				AssertThat(*ptr, Is().Not().EqualTo(nullptr));

				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(*ptr, Equals(nullptr));
			});
		});

		describe("Comparisons", []() {
			it("Owner can equal Owner", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				PtrOwner<EmptyStruct> ownerEmpty;

				AssertThat(owner == owner, Equals(true));
				AssertThat(owner == owner2, Equals(false));
				AssertThat(ownerEmpty == ownerEmpty, Equals(true));
				AssertThat(owner == ownerEmpty, Equals(false));

				AssertThat(owner != owner, Equals(false));
				AssertThat(owner != owner2, Equals(true));
				AssertThat(ownerEmpty != ownerEmpty, Equals(false));
				AssertThat(owner != ownerEmpty, Equals(true));
			});

			it("Owner can equal Weak", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak = owner.AsPtr();
				PtrOwner<EmptyStruct> ownerEmpty;
				Ptr<EmptyStruct> weakEmpty;

				AssertThat(owner == weak, Equals(true));
				AssertThat(owner2 == weak, Equals(false));
				AssertThat(ownerEmpty == weak, Equals(false));
				AssertThat(ownerEmpty == weakEmpty, Equals(true));

				AssertThat(owner != weak, Equals(false));
				AssertThat(owner2 != weak, Equals(true));
				AssertThat(ownerEmpty != weak, Equals(true));
				AssertThat(ownerEmpty != weakEmpty, Equals(false));
			});

			it("Weak can equal Weak", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak = owner.AsPtr();
				auto weak2 = owner2.AsPtr();
				Ptr<EmptyStruct> weakEmpty;

				AssertThat(weak == weak, Equals(true));
				AssertThat(weak2 == weak, Equals(false));
				AssertThat(weakEmpty == weak, Equals(false));
				AssertThat(weakEmpty == weakEmpty, Equals(true));

				AssertThat(weak != weak, Equals(false));
				AssertThat(weak2 != weak, Equals(true));
				AssertThat(weakEmpty != weak, Equals(true));
				AssertThat(weakEmpty != weakEmpty, Equals(false));
			});

			it("Weak can equal Owner", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak = owner.AsPtr();
				auto weak2 = owner2.AsPtr();
				PtrOwner<EmptyStruct> ownerEmpty;
				Ptr<EmptyStruct> weakEmpty;

				AssertThat(weak == owner, Equals(true));
				AssertThat(weak2 == owner, Equals(false));
				AssertThat(weakEmpty == owner, Equals(false));
				AssertThat(weakEmpty == ownerEmpty, Equals(true));

				AssertThat(weak != owner, Equals(false));
				AssertThat(weak2 != owner, Equals(true));
				AssertThat(weakEmpty != owner, Equals(true));
				AssertThat(weakEmpty != ownerEmpty, Equals(false));
			});
		});

		describe("Counter", []() {
			it("Adds weaks", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				const auto* counter = owner.GetCounter();
				AssertThat(counter->weaks.load(), Equals(0u));

				auto weak = owner.AsPtr();
				AssertThat(counter->weaks.load(), Equals(1u));
			});

			it("Removes weaks", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				const auto* counter = owner.GetCounter();
				{
					auto weak = owner.AsPtr();
					AssertThat(counter->weaks.load(), Equals(1u));
				}
				AssertThat(counter->weaks.load(), Equals(0u));
			});

			it("Removes with owner release", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				AssertThat(owner.GetCounter(), Is().Not().EqualTo(nullptr));

				owner.Release();
				AssertThat(owner.GetCounter(), Equals(nullptr));
			});

			it("Removes with no weaks left", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto weak = owner.AsPtr();
				AssertThat(weak.GetCounter(), Is().Not().EqualTo(nullptr));

				owner.Release();
				AssertThat(weak.GetCounter(), Is().Not().EqualTo(nullptr));

				weak.Reset();
				AssertThat(owner.GetCounter(), Equals(nullptr));
			});
		});
	});
});

inline bool MockStruct::bCalledDelete = false;
