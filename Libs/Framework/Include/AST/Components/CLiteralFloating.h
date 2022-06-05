// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Core/EnumFlags.h>
#include <Reflection/Reflection.h>


namespace rift
{
	using namespace p;
	using namespace p::EnumOperators;

	enum class FloatingType : u8
	{
		F32 = 32,
		F64 = 64
	};
}    // namespace rift
ENUM(rift::FloatingType)


namespace rift
{
	struct CLiteralFloating : public CLiteral
	{
		STRUCT(CLiteralFloating, CLiteral)


		PROP(value)
		double value = 0.;

		PROP(type)
		FloatingType type = FloatingType::F32;


		u8 GetSize() const
		{
			return *type;
		}
	};
}    // namespace rift
