// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

// Lives in Include/Std so no header name here can shadow a system header on case
// insensitive filesystems (<math.h>, <io.h>, <stdio.h>), which breaks <cmath>/<cstdio>
// on macOS and Windows.
#include <Pipe/Core/StringView.h>

/**
 * Mirrors the StringView struct the MIR backend emits for Rift's String type
 * (see CGenerator::DeclareNativeTypes). Members must stay in this order.
 *
 * TODO: Take p::StringView directly once Pipe defines its own string view instead of
 * aliasing std::string_view. std::string_view's member order is implementation defined
 * (libstdc++ is { size, data }, libc++ and MSVC are { data, size }), so mirroring it in
 * generated C would tie Rift's ABI to the host standard library.
 */
struct StringView
{
	const char* data;
	p::sizet size;
};

// Rift functions always return void: inputs are passed by value, outputs by pointer.
extern "C"
{
	void Print(StringView text);
	void PrintLine(StringView text);
}
