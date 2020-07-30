
#include <Pointers/PtrOwner.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace VCLang;

struct EmptyStruct
{};


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

			it("Owner is released when destroyed", [&]() {
				Ptr2<EmptyStruct> ptr;
				{
					PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
					ptr = owner;
					AssertThat(ptr.IsValid(), Equals(true));
				}
				AssertThat(ptr.IsValid(), Equals(false));
			});
		});

		describe("Weak pointer", []() {
			it("Can initialize to empty", [&]() {
				Ptr2<EmptyStruct> ptr;
				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.Get(), Equals(nullptr));
			});

			it("Can initialize from owner", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				Ptr2<EmptyStruct> ptr = owner;

				AssertThat(ptr.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Is().Not().EqualTo(nullptr));
			});

			it("Can initialize from other weak", [&]() {
				PtrOwner<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				Ptr2<EmptyStruct> ptr = owner;
				Ptr2<EmptyStruct> ptr2 = ptr;

				AssertThat(ptr2.IsValid(), Equals(true));
				AssertThat(ptr2.Get(), Is().Not().EqualTo(nullptr));
			});
		});
	});
});
