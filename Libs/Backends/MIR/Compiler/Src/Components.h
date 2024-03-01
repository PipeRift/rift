// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <mir.h>
#include <Pipe/Reflect/Struct.h>


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
