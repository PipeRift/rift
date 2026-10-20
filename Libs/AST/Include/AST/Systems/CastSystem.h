// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// In which ways a cast between two types is allowed
	enum class CastFlags : p::u8
	{
		None = 0,
		// Applied automatically when a pin of one type receives a value of another type
		Implicit = 1 << 0,
		// Requires an explicit cast node
		Explicit = 1 << 1,
		// Both implicit and explicit
		All = Implicit | Explicit
	};

	constexpr CastFlags operator|(CastFlags lhs, CastFlags rhs)
	{
		return static_cast<CastFlags>(p::u8(lhs) | p::u8(rhs));
	}

	constexpr CastFlags operator&(CastFlags lhs, CastFlags rhs)
	{
		return static_cast<CastFlags>(p::u8(lhs) & p::u8(rhs));
	}

	constexpr CastFlags& operator|=(CastFlags& lhs, CastFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr bool HasFlag(CastFlags flags, CastFlags flag)
	{
		return (flags & flag) == flag;
	}


	// How a backend produces the cast value. Mirrors the kinds of cast instructions
	// LLVM uses, so a backend can pick the instruction that matches the cast
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
	struct Cast
	{
		Id fromType     = NoId;
		Id toType       = NoId;
		CastKind kind   = CastKind::IntCast;
		CastFlags flags = CastFlags::None;
	};


	// Casts registered in a tree. Transient: it's rebuilt when the tree is created
	struct SCasts
	{
		P_STRUCT(SCasts)

		p::TArray<Cast> casts;
	};
}    // namespace rift::ast


namespace rift::ast::CastSystem
{
	// Adds a cast, replacing any previous cast between the same types
	void RegisterCast(Tree& ast, const Cast& cast);

	// Registers the casts between the built-in numeric types, following the C++
	// rules: casts that keep every value of the source type are implicit, the
	// rest (narrowing ones) need an explicit cast node
	void RegisterNumericCasts(Tree& ast);

	// The cast between two types, or nullptr when they can't be cast into each other
	const Cast* FindCast(const Tree& ast, Id fromType, Id toType);

	// Every cast registered in the tree
	p::TView<const Cast> GetCasts(const Tree& ast);

	// Can a value of fromType become a toType with casts supporting all the required
	// flags?
	bool CanCast(const Tree& ast, Id fromType, Id toType, CastFlags required);

	// Can a value of fromType become a toType without an explicit cast node?
	bool CanCastImplicitly(const Tree& ast, Id fromType, Id toType);

	// Can a cast node turn a fromType into a toType? Explicit casts also
	// perform the implicit ones
	bool CanCastExplicitly(const Tree& ast, Id fromType, Id toType);
}    // namespace rift::ast::CastSystem
