// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Pipe/Core/EnumFlags.h>
#include <Pipe/Reflect/Reflection.h>


namespace rift::AST
{
	using namespace p::core;

	enum class FloatingType : u8
	{
		F32 = 32,
		F64 = 64
	};
}    // namespace rift::AST
ENUM(rift::AST::FloatingType)


namespace rift::AST
{
	struct CLiteralFloating : public CLiteral
	{
		P_STRUCT(CLiteralFloating, CLiteral)


		P_PROP(value)
		double value = 0.;

		P_PROP(type)
		FloatingType type = FloatingType::F32;


		u8 GetSize() const
		{
			return static_cast<UnderlyingType<FloatingType>>(type);
		}
	};
}    // namespace rift::AST
