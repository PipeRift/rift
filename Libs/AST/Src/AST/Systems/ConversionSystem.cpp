// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/ConversionSystem.h"

#include <PipeECS.h>


namespace rift::ast::ConversionSystem
{
	namespace
	{
		// Traits of a built-in numeric type, used to compute the conversions between them
		struct NumericType
		{
			Id id         = NoId;
			bool isBool   = false;
			bool isFloat  = false;
			bool isSigned = false;
			p::u8 size    = 0;    // Size in bytes
		};

		// Number of bits of mantissa a float keeps, which limits the integers it can
		// represent exactly
		constexpr p::u8 floatMantissaBits  = 24;
		constexpr p::u8 doubleMantissaBits = 53;

		// Can every value of 'from' be represented exactly in 'to'?
		bool IsValuePreserving(const NumericType& from, const NumericType& to)
		{
			if (from.id == to.id)
			{
				return true;
			}
			if (from.isBool)
			{
				// False and true fit in every numeric type
				return true;
			}
			if (to.isFloat)
			{
				if (from.isFloat)
				{
					// Only floats can hold all the values of smaller floats
					return from.size < to.size;
				}
				// Integers need one more bit than their signed range to be held exactly
				const p::u8 mantissaBits = to.size == 4 ? floatMantissaBits : doubleMantissaBits;
				const p::u8 valueBits    = p::u8(from.size * 8 - (from.isSigned ? 1 : 0));
				return valueBits <= mantissaBits;
			}
			if (from.isFloat)
			{
				// Integers can't hold the fractional part of a float
				return false;
			}
			if (from.isSigned == to.isSigned)
			{
				// Same signedness: the target must be at least as wide
				return to.size >= from.size;
			}
			// Unsigned values fit in a signed type when it's strictly wider
			return !from.isSigned && to.size > from.size;
		}

		// The kind of cast that turns one numeric type into another
		CastKind GetNumericCastKind(const NumericType& from, const NumericType& to)
		{
			if (!from.isFloat && !to.isFloat)
			{
				// Bool is an integer too
				return CastKind::IntCast;
			}
			if (from.isFloat && to.isFloat)
			{
				return CastKind::FPToFP;
			}
			return from.isFloat ? CastKind::FPToInt : CastKind::IntToFP;
		}

		p::TArray<NumericType> GetNumericTypes(const NativeTypeIds& natives)
		{
			return {
			    {natives.boolId,   true,  false, false, 1},
			    {natives.i8Id,     false, false, true,  1},
			    {natives.u8Id,     false, false, false, 1},
			    {natives.i16Id,    false, false, true,  2},
			    {natives.u16Id,    false, false, false, 2},
			    {natives.i32Id,    false, false, true,  4},
			    {natives.u32Id,    false, false, false, 4},
			    {natives.i64Id,    false, false, true,  8},
			    {natives.u64Id,    false, false, false, 8},
			    {natives.floatId,  false, true,  true,  4},
			    {natives.doubleId, false, true,  true,  8},
			};
		}
	}    // namespace


	void RegisterConversion(Tree& ast, const Conversion& conversion)
	{
		auto& conversions = ast.GetOrSetStatic<SConversions>().conversions;
		for (Conversion& existing : conversions)
		{
			if (existing.fromType == conversion.fromType && existing.toType == conversion.toType)
			{
				existing = conversion;
				return;
			}
		}
		conversions.Add(conversion);
	}

	void RegisterNumericConversions(Tree& ast)
	{
		const p::TArray<NumericType> types = GetNumericTypes(ast.GetNativeTypes());
		for (const NumericType& from : types)
		{
			if (p::IsNone(from.id))
			{
				continue;
			}
			for (const NumericType& to : types)
			{
				if (p::IsNone(to.id) || from.id == to.id)
				{
					continue;
				}
				// Conversions that keep every value of the source type happen on their own,
				// the rest need an explicit cast node
				const ConversionFlags flags =
				    IsValuePreserving(from, to) ? ConversionFlags::All : ConversionFlags::Explicit;
				RegisterConversion(ast, {.fromType  = from.id,
				                            .toType = to.id,
				                            .kind   = GetNumericCastKind(from, to),
				                            .flags  = flags});
			}
		}
	}

	const Conversion* FindConversion(const Tree& ast, Id fromType, Id toType)
	{
		if (const auto* registry = ast.TryGetStatic<SConversions>())
		{
			for (const Conversion& conversion : registry->conversions)
			{
				if (conversion.fromType == fromType && conversion.toType == toType)
				{
					return &conversion;
				}
			}
		}
		return nullptr;
	}

	p::TView<const Conversion> GetConversions(const Tree& ast)
	{
		if (const auto* registry = ast.TryGetStatic<SConversions>())
		{
			return registry->conversions;
		}
		return {};
	}

	bool CanConvert(const Tree& ast, Id fromType, Id toType, ConversionFlags required)
	{
		const Conversion* conversion = FindConversion(ast, fromType, toType);
		return conversion != nullptr && HasFlag(conversion->flags, required);
	}

	bool CanConvertImplicitly(const Tree& ast, Id fromType, Id toType)
	{
		return CanConvert(ast, fromType, toType, ConversionFlags::Implicit);
	}

	bool CanConvertExplicitly(const Tree& ast, Id fromType, Id toType)
	{
		const Conversion* conversion = FindConversion(ast, fromType, toType);
		return conversion != nullptr && conversion->flags != ConversionFlags::None;
	}
}    // namespace rift::ast::ConversionSystem
