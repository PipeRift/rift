// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <mir.h>
#include <Pipe/Core/Tag.h>


namespace rift
{
	struct CMIRModule
	{
		p::String code;
	};
	struct CMIRType
	{
		p::Tag value;
	};
	struct CMIRLiteral
	{
		p::Tag value;
	};
	struct CMIRFunctionSignature
	{
		p::String value;
	};
}    // namespace rift
