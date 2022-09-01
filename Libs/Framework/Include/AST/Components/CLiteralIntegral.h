// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CLiteral.h"

#include <Pipe/Core/EnumFlags.h>
#include <Pipe/Reflect/Reflection.h>


namespace rift::AST
{
	using namespace p::core;

	static constexpr u8 literalUnsignedMask = 1 << 7;    // Last bit marks type as unsigned

	enum class IntegralType : u8
	{
		S8  = 8,
		S16 = 16,
		S32 = 32,
		S64 = 64,
		U8  = S8 | literalUnsignedMask,
		U16 = S16 | literalUnsignedMask,
		U32 = S32 | literalUnsignedMask,
		U64 = S64 | literalUnsignedMask
	};
}    // namespace rift::AST
ENUM(rift::AST::IntegralType)
template<>
struct magic_enum::customize::enum_range<rift::AST::IntegralType>
{
	static constexpr int min = 0;
	static constexpr int max = 256;
	// static constexpr bool is_flags = true;
	//  (max - min) must be less than UINT16_MAX.
};


namespace rift::AST
{
	struct CLiteralIntegral : public CLiteral
	{
		STRUCT(CLiteralIntegral, CLiteral)


		PROP(value)
		u64 value = 0;

		PROP(type)
		IntegralType type = IntegralType::S32;


		bool IsSigned() const
		{
			return u8(type) & literalUnsignedMask == literalUnsignedMask;
		}
		u8 GetSize() const
		{
			return u8(type) & ~literalUnsignedMask;
		}
	};
}    // namespace rift::AST
