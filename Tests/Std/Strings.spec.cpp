// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Std/Strings.h>

#include <string_view>


using namespace p;


namespace
{
	::StringView MakeView(p::StringView text)
	{
		return ::StringView{text.data(), text.size()};
	}

	std::string_view ToText(const ::String& text)
	{
		return {text.data, text.size};
	}
}    // namespace


P_SPEC("Std.Strings", []()
{
	It("Measures the length of a view", []()
	{
		unsigned long long length = 0;
		StringLength(MakeView("Hello"), &length);
		Expect(length).ToEqual(5ull);
	});

	It("Takes substrings of a view without copying it", []()
	{
		const ::StringView text = MakeView("Hello World");
		::StringView substring{};
		StringSubstring(text, 6, 5, &substring);

		Expect(substring.size).ToEqual(5ull);
		Expect(std::string_view{substring.data, substring.size}).ToEqual(std::string_view{"World"});
		// The substring points into the original buffer
		Expect(substring.data == text.data + 6).ToBeTrue();
	});

	It("Finds the position of a substring", []()
	{
		long long position = 0;
		StringFind(MakeView("Hello World"), MakeView("World"), &position);
		Expect(position).ToEqual(6ll);

		StringFind(MakeView("Hello World"), MakeView("Nope"), &position);
		Expect(position).ToEqual(-1ll);
	});

	It("Checks if a view contains a substring", []()
	{
		char contains = 0;
		StringContains(MakeView("Hello World"), MakeView("lo W"), &contains);
		Expect(contains).ToEqual(1);

		StringContains(MakeView("Hello World"), MakeView("nope"), &contains);
		Expect(contains).ToEqual(0);
	});

	It("Compares views", []()
	{
		char equals = 0;
		StringEquals(MakeView("Hello"), MakeView("Hello"), &equals);
		Expect(equals).ToEqual(1);

		StringEquals(MakeView("Hello"), MakeView("Hell"), &equals);
		Expect(equals).ToEqual(0);
	});

	It("Checks the ends of a view", []()
	{
		char matches = 0;
		StringStartsWith(MakeView("Hello World"), MakeView("Hello"), &matches);
		Expect(matches).ToEqual(1);

		StringEndsWith(MakeView("Hello World"), MakeView("World"), &matches);
		Expect(matches).ToEqual(1);

		StringStartsWith(MakeView("Hello World"), MakeView("World"), &matches);
		Expect(matches).ToEqual(0);
	});

	It("Concatenates views into a new string", []()
	{
		::String result{};
		StringConcat(MakeView("Hello"), MakeView(" World"), &result);

		Expect(ToText(result)).ToEqual(std::string_view{"Hello World"});
	});

	It("Repeats a view into a new string", []()
	{
		::String result{};
		StringRepeat(MakeView("ab"), 3, &result);

		Expect(ToText(result)).ToEqual(std::string_view{"ababab"});
	});

	It("Appends to a string", []()
	{
		::String first{};
		StringConcat(MakeView("Hello"), MakeView(""), &first);

		::String result{};
		StringAppend(first, MakeView(" World"), &result);

		Expect(ToText(result)).ToEqual(std::string_view{"Hello World"});
	});

	It("Replaces parts of a string", []()
	{
		::String first{};
		StringConcat(MakeView("Hello World"), MakeView(""), &first);

		::String result{};
		StringReplace(first, MakeView("World"), MakeView("There"), &result);

		Expect(ToText(result)).ToEqual(std::string_view{"Hello There"});
	});

	It("Measures the size of a string", []()
	{
		::String text{};
		StringConcat(MakeView("Hello"), MakeView(" World"), &text);

		unsigned long long size = 0;
		StringSize(text, &size);
		Expect(size).ToEqual(11ull);
	});
});
