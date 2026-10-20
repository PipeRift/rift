// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Std/String.h"

/**
 * String and string view operations.
 *
 * String views only support the operations that can be done in place, while strings also
 * support the operations that need to allocate a new buffer.
 *
 * Rift functions always return void: inputs are passed by value, outputs by pointer.
 * Rift's Bool type is generated as a char, so predicates write 0 or 1 to their output.
 */
extern "C"
{
	// String views: operations done in place
	void StringLength(StringView text, unsigned long long* out);
	void StringSubstring(StringView text, unsigned long long start, unsigned long long count,
	    StringView* out);
	void StringFind(StringView text, StringView needle, long long* out);
	void StringContains(StringView text, StringView needle, char* out);
	void StringEquals(StringView left, StringView right, char* out);
	void StringStartsWith(StringView text, StringView prefix, char* out);
	void StringEndsWith(StringView text, StringView suffix, char* out);

	// Strings: operations that need to allocate a new buffer
	void StringConcat(StringView left, StringView right, String* out);
	void StringRepeat(StringView text, unsigned long long count, String* out);
	void StringAppend(String text, StringView extra, String* out);
	void StringReplace(String text, StringView target, StringView replacement, String* out);
	void StringSize(String text, unsigned long long* out);
}
