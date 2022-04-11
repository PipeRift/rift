// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Misc/EnumFlags.h>
#include <Reflection/Reflection.h>


namespace Rift
{
	using namespace Rift::EnumOperators;

	enum class IntegralType : u8
	{
		Unsigned = 1 << 7,    // Last bit marks type as unsigned
		S8       = 8,
		S16      = 16,
		S32      = 32,
		S64      = 64,
		U8       = S8 | Unsigned,
		U16      = S16 | Unsigned,
		U32      = S32 | Unsigned,
		U64      = S64 | Unsigned
	};
	ENUM(IntegralType)


	struct CLiteralIntegral : public CLiteral
	{
		STRUCT(CLiteralIntegral, CLiteral)


		PROP(value)
		u64 value = 0;

		PROP(type)
		IntegralType type = IntegralType::S32;


		bool IsSigned() const
		{
			return !HasFlag(type, IntegralType::Unsigned);
		}
		u8 GetSize() const
		{
			IntegralType size = type;
			RemoveFlags(size, IntegralType::Unsigned);
			return *size;
		}
	};
}    // namespace Rift
