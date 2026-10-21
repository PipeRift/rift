// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// How a backend produces the cast value. Mirrors the kinds of cast instructions
	// LLVM uses, so a backend can pick the instruction that matches the cast
	enum class CastKind : p::u8
	{
		// Integer to integer, including bool: extension, truncation or sign change
		IntToInt,
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
		Id fromType   = NoId;
		Id toType     = NoId;
		CastKind kind = CastKind::IntToInt;

		// Casts are sorted by the types they go between, so they can be looked up in bulk
		constexpr bool operator<(const Cast& other) const
		{
			return fromType != other.fromType ? fromType < other.fromType : toType < other.toType;
		}
		constexpr bool operator==(const Cast& other) const
		{
			return fromType == other.fromType && toType == other.toType;
		}
	};


	// Casts registered in a tree. Transient: it's rebuilt when the tree is created
	struct SCasts
	{
		P_STRUCT(SCasts)

		// Casts a cast node can perform, including the implicit ones
		p::TArray<Cast> explicitCasts;
		// Casts a value can go through on its own
		p::TArray<Cast> implicitCasts;
	};
}    // namespace rift::ast


namespace rift::ast::CastSystem
{
	// Adds a cast. Implicit casts are also applied when a pin receives the value
	void RegisterCast(Tree& ast, const Cast& cast, bool implicit);

	// Registers the casts between the built-in numeric types, following the C++
	// rules: casts that keep every value of the source type are implicit, the
	// rest (narrowing ones) need an explicit cast node
	void RegisterNumericCasts(Tree& ast);

	// The cast a value can go through on its own, or nullptr when there is none
	const Cast* FindCastImplicit(const p::IdContext& ast, Id fromType, Id toType);
	// The cast a cast node can perform, or nullptr when there is none
	const Cast* FindCast(const p::IdContext& ast, Id fromType, Id toType);

	// Every cast a cast node can perform
	p::TView<const Cast> GetCasts(const p::IdContext& ast);

	// Can a value of fromType become a toType without an explicit cast node?
	bool CanCastImplicit(p::TIdScopeRef<> scope, Id fromType, Id toType);

	// Can a cast node turn a fromType into a toType? Explicit casts also
	// perform the implicit ones
	bool CanCast(p::TIdScopeRef<> scope, Id fromType, Id toType);
}    // namespace rift::ast::CastSystem
