// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Std/Strings.h"

#include <PipeStrings.h>

#include <cstring>


namespace
{
	p::StringView ToView(StringView view)
	{
		return {view.data, view.size};
	}

	/**
	 * Copies the given contents into a buffer owned by the runtime.
	 * The buffer lives in the current arena, so it is released with it.
	 */
	String ToOwned(p::StringView contents)
	{
		String result{};
		if (!contents.empty())
		{
			result.data = static_cast<char*>(p::Alloc(contents.size()));
			std::memcpy(result.data, contents.data(), contents.size());
			result.size = contents.size();
		}
		return result;
	}
}    // namespace


extern "C"
{
	void StringLength(StringView text, unsigned long long* out)
	{
		*out = ToView(text).size();
	}

	void StringSubstring(
	    StringView text, unsigned long long start, unsigned long long count, StringView* out)
	{
		const p::StringView view = ToView(text);
		if (start >= view.size())
		{
			*out = StringView{nullptr, 0};
			return;
		}
		const p::StringView substring = view.substr(start, count);
		*out                          = StringView{substring.data(), substring.size()};
	}

	void StringFind(StringView text, StringView needle, long long* out)
	{
		const p::sizet found = p::Strings::Find(ToView(text), ToView(needle));
		*out                 = found == p::StringView::npos ? -1 : static_cast<long long>(found);
	}

	void StringContains(StringView text, StringView needle, char* out)
	{
		*out = p::Strings::Contains(ToView(text), ToView(needle)) ? 1 : 0;
	}

	void StringEquals(StringView left, StringView right, char* out)
	{
		*out = p::Strings::Equals(ToView(left), ToView(right)) ? 1 : 0;
	}

	void StringStartsWith(StringView text, StringView prefix, char* out)
	{
		*out = p::Strings::StartsWith(ToView(text), ToView(prefix)) ? 1 : 0;
	}

	void StringEndsWith(StringView text, StringView suffix, char* out)
	{
		*out = p::Strings::EndsWith(ToView(text), ToView(suffix)) ? 1 : 0;
	}

	void StringConcat(StringView left, StringView right, String* out)
	{
		p::String result;
		result.append(ToView(left));
		result.append(ToView(right));
		*out = ToOwned(result);
	}

	void StringRepeat(StringView text, unsigned long long count, String* out)
	{
		const p::StringView view = ToView(text);
		p::String result;
		for (unsigned long long i = 0; i < count; ++i)
		{
			result.append(view);
		}
		*out = ToOwned(result);
	}

	void StringAppend(String text, StringView extra, String* out)
	{
		p::String result;
		result.append(p::StringView{text.data, text.size});
		result.append(ToView(extra));
		*out = ToOwned(result);
	}

	void StringReplace(String text, StringView target, StringView replacement, String* out)
	{
		const p::StringView source{text.data, text.size};
		const p::StringView from = ToView(target);
		const p::StringView to   = ToView(replacement);

		p::String result;
		if (from.empty())
		{
			result.append(source);
			*out = ToOwned(result);
			return;
		}

		p::sizet position = 0;
		while (true)
		{
			const p::sizet found = source.find(from, position);
			if (found == p::StringView::npos)
			{
				result.append(source.substr(position));
				break;
			}
			result.append(source.substr(position, found - position));
			result.append(to);
			position = found + from.size();
		}
		*out = ToOwned(result);
	}

	void StringSize(String text, unsigned long long* out)
	{
		*out = text.size;
	}
}
