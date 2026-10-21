// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Id.h"
#include "AST/Tree.h"

#include <PipeContainers.h>
#include <PipeReflect.h>


namespace rift::ast
{
	// Describes how a value of one type becomes a value of another
	struct Cast
	{
		Id fromType = NoId;
		Id toType   = NoId;

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

	void RegisterNumericCasts(Tree& ast);

	const Cast* FindCastImplicit(const p::IdContext& ast, Id fromType, Id toType);
	const Cast* FindCast(const p::IdContext& ast, Id fromType, Id toType);

	// Can a value of fromType become a toType without an explicit cast node?
	bool CanCastImplicit(p::TIdScopeRef<> scope, Id fromType, Id toType);

	bool CanCast(p::TIdScopeRef<> scope, Id fromType, Id toType);

	p::TView<const Cast> GetCasts(const p::IdContext& ast);
}    // namespace rift::ast::CastSystem
