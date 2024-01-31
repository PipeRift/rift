// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/EnumFlags.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	enum class FloatingType : p::u8
	{
		F32 = 32,
		F64 = 64
	};


	static constexpr p::u8 literalUnsignedMask = 1 << 7;    // Last bit marks type as unsigned
	enum class IntegralType : p::u8
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

ENUM(rift::AST::FloatingType)
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
	struct CLiteralBool : public p::Struct
	{
		P_STRUCT(CLiteralBool, p::Struct)

		P_PROP(value)
		bool value = false;
	};


	struct CLiteralFloating : public p::Struct
	{
		P_STRUCT(CLiteralFloating, p::Struct)


		P_PROP(value)
		double value = 0.;

		P_PROP(type)
		FloatingType type = FloatingType::F32;


		p::u8 GetSize() const
		{
			return static_cast<p::UnderlyingType<FloatingType>>(type);
		}
	};


	struct CLiteralIntegral : public p::Struct
	{
		P_STRUCT(CLiteralIntegral, p::Struct)


		P_PROP(value)
		p::u64 value = 0;

		P_PROP(type)
		IntegralType type = IntegralType::S32;


		bool IsSigned() const
		{
			return p::u8(type) & literalUnsignedMask == literalUnsignedMask;
		}
		p::u8 GetSize() const
		{
			return p::u8(type) & ~literalUnsignedMask;
		}
	};


	struct CLiteralString : public p::Struct
	{
		P_STRUCT(CLiteralString, p::Struct)

		P_PROP(value)
		p::String value;
	};
}    // namespace rift::AST
