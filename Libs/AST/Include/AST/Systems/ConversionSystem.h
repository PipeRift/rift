// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// In which ways a conversion between two types is allowed
	enum class ConversionFlags : p::u8
	{
		None = 0,
		// Applied automatically when a pin of one type receives a value of another type
		Implicit = 1 << 0,
		// Requires an explicit cast node
		Explicit = 1 << 1,
		// Both implicit and explicit
		All = Implicit | Explicit
	};

	constexpr ConversionFlags operator|(ConversionFlags lhs, ConversionFlags rhs)
	{
		return static_cast<ConversionFlags>(p::u8(lhs) | p::u8(rhs));
	}

	constexpr ConversionFlags operator&(ConversionFlags lhs, ConversionFlags rhs)
	{
		return static_cast<ConversionFlags>(p::u8(lhs) & p::u8(rhs));
	}

	constexpr ConversionFlags& operator|=(ConversionFlags& lhs, ConversionFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr bool HasFlag(ConversionFlags flags, ConversionFlags flag)
	{
		return (flags & flag) == flag;
	}


	// How a backend produces the converted value. Mirrors the kinds of cast instructions
	// LLVM uses, so a backend can pick the instruction that matches the conversion
	enum class CastKind : p::u8
	{
		// Integer to integer, including bool: extension, truncation or sign change
		IntCast,
		// Floating point to floating point: extension or truncation
		FPToFP,
		// Integer to floating point
		IntToFP,
		// Floating point to integer
		FPToInt
	};


	// Describes how a value of one type becomes a value of another
	struct Conversion
	{
		Id fromType           = NoId;
		Id toType             = NoId;
		CastKind kind         = CastKind::IntCast;
		ConversionFlags flags = ConversionFlags::None;
	};


	// Conversions registered in a tree. Transient: it's rebuilt when the tree is created
	struct SConversions
	{
		P_STRUCT(SConversions)

		p::TArray<Conversion> conversions;
	};
}    // namespace rift::ast


namespace rift::ast::ConversionSystem
{
	// Adds a conversion, replacing any previous conversion between the same types
	void RegisterConversion(Tree& ast, const Conversion& conversion);

	// Registers the conversions between the built-in numeric types, following the C++
	// rules: conversions that keep every value of the source type are implicit, the
	// rest (narrowing ones) need an explicit cast node
	void RegisterNumericConversions(Tree& ast);

	// The conversion between two types, or nullptr when they can't convert into each other
	const Conversion* FindConversion(const Tree& ast, Id fromType, Id toType);

	// Every conversion registered in the tree
	p::TView<const Conversion> GetConversions(const Tree& ast);

	// Can a value of fromType become a toType with conversions supporting all the required
	// flags?
	bool CanConvert(const Tree& ast, Id fromType, Id toType, ConversionFlags required);

	// Can a value of fromType become a toType without an explicit cast node?
	bool CanConvertImplicitly(const Tree& ast, Id fromType, Id toType);

	// Can a cast node turn a fromType into a toType? Explicit conversions also
	// perform the implicit ones
	bool CanConvertExplicitly(const Tree& ast, Id fromType, Id toType);
}    // namespace rift::ast::ConversionSystem
