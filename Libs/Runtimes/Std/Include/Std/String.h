// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <Pipe/Core/StringView.h>

/**
 * Mirrors the StringView struct the MIR backend emits for Rift's StringView type
 * (see CGenerator::DeclareNativeTypes). Members must stay in this order and keep the
 * same size type the backend generates.
 *
 * String views borrow their contents, so they can only be used with operations that do
 * not need to modify or own the data they point to.
 *
 * TODO: Take p::StringView directly once Pipe defines its own string view instead of
 * aliasing std::string_view. std::string_view's member order is implementation defined
 * (libstdc++ is { size, data }, libc++ and MSVC are { data, size }), so mirroring it in
 * generated C would tie Rift's ABI to the host standard library.
 */
struct StringView
{
	const char* data;
	unsigned long long size;
};

/**
 * Mirrors the String struct the MIR backend emits for Rift's String type
 * (see CGenerator::DeclareNativeTypes). Members must stay in this order.
 *
 * Strings own their contents, so they can be used with the operations that need to
 * allocate a new buffer. The buffer is owned by the runtime and lives as long as the
 * arena it was allocated from.
 */
struct String
{
	char* data;
	unsigned long long size;
};
