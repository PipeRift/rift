// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/CastSystem.h"

#include <PipeECS.h>


namespace rift::ast::CastSystem
{
	namespace
	{
		// Traits of a built-in numeric type, used to compute the casts between them
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

		// Adds the cast keeping the array sorted by its types, replacing an existing one
		void AddSorted(p::TArray<Cast>& casts, const Cast& cast)
		{
			const p::i32 index = casts.FindSorted(cast);
			if (index != p::NO_INDEX)
			{
				casts[index] = cast;
				return;
			}
			casts.AddSorted(cast);
		}

		const Cast* Find(const p::TArray<Cast>& casts, Id fromType, Id toType)
		{
			const Cast type{.fromType = fromType, .toType = toType};
			const p::i32 index = casts.FindSorted(type);
			return index != p::NO_INDEX ? &casts[index] : nullptr;
		}
	}    // namespace


	void RegisterCast(Tree& ast, const Cast& cast, CastFlags flags)
	{
		auto& registry = ast.GetOrSetStatic<SCasts>();
		AddSorted(registry.explicitCasts, cast);
		if (p::HasFlag(flags, CastFlags::Implicit))
		{
			AddSorted(registry.implicitCasts, cast);
		}
	}

	void RegisterNumericCasts(Tree& ast)
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
				// Casts that keep every value of the source type happen on their own,
				// the rest need an explicit cast node
				const CastFlags flags =
				    IsValuePreserving(from, to) ? CastFlags::All : CastFlags::Explicit;
				RegisterCast(ast,
				    {.fromType = from.id, .toType = to.id, .kind = GetNumericCastKind(from, to)},
				    flags);
			}
		}
	}

	const Cast* FindImplicitCast(const p::IdContext& ast, Id fromType, Id toType)
	{
		if (const auto* registry = ast.TryGetStatic<SCasts>())
		{
			return Find(registry->implicitCasts, fromType, toType);
		}
		return nullptr;
	}

	const Cast* FindExplicitCast(const p::IdContext& ast, Id fromType, Id toType)
	{
		if (const auto* registry = ast.TryGetStatic<SCasts>())
		{
			return Find(registry->explicitCasts, fromType, toType);
		}
		return nullptr;
	}

	p::TView<const Cast> GetCasts(const p::IdContext& ast)
	{
		if (const auto* registry = ast.TryGetStatic<SCasts>())
		{
			return registry->explicitCasts;
		}
		return {};
	}

	bool CanCastImplicitly(p::TIdScopeRef<> scope, Id fromType, Id toType)
	{
		return FindImplicitCast(scope.GetContext(), fromType, toType) != nullptr;
	}

	bool CanCastExplicitly(p::TIdScopeRef<> scope, Id fromType, Id toType)
	{
		return FindExplicitCast(scope.GetContext(), fromType, toType) != nullptr;
	}
}    // namespace rift::ast::CastSystem
