// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Misc/EnumFlags.h>
#include <Reflection/Reflection.h>


namespace Rift
{
	using namespace Rift::EnumOperators;

	enum class FloatingType : u8
	{
		F32 = 32,
		F64 = 64
	};
	ENUM(FloatingType)


	struct CLiteralFloating : public CLiteral
	{
		STRUCT(CLiteralFloating, CLiteral)


		PROP(value)
		float value = 0.f;

		PROP(type)
		FloatingType type = FloatingType::F32;


		u8 GetSize() const
		{
			return *type;
		}
	};
}    // namespace Rift
