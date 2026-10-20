// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Std/String.h"

/**
 * Rift functions always return void: inputs are passed by value, outputs by pointer.
 */
extern "C"
{
	void Print(StringView text);
	void PrintLine(StringView text);
}
