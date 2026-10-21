// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/CastSystem.h"

#include <PipeECS.h>


namespace rift::ast::CastSystem
{
	namespace
	{
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
		const NativeTypeIds& natives = ast.GetNativeTypes();

		// Casts that keep every value of the source type, so they happen on their own.
		// The built-in numeric types follow the C++ rules: widening integers and floats,
		// integers that fit a float's mantissa, and bool to any number
		const Cast implicitNumericCasts[]{
		    {natives.boolId,  natives.i8Id,     CastKind::IntToInt},
		    {natives.boolId,  natives.u8Id,     CastKind::IntToInt},
		    {natives.boolId,  natives.i16Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.u16Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.i32Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.u32Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.i64Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.u64Id,    CastKind::IntToInt},
		    {natives.boolId,  natives.floatId,  CastKind::IntToFP },
		    {natives.boolId,  natives.doubleId, CastKind::IntToFP },

		    {natives.i8Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.i8Id,    natives.i32Id,    CastKind::IntToInt},
		    {natives.i8Id,    natives.i64Id,    CastKind::IntToInt},
		    {natives.i8Id,    natives.floatId,  CastKind::IntToFP },
		    {natives.i8Id,    natives.doubleId, CastKind::IntToFP },

		    {natives.u8Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.i32Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.u32Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.i64Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.u64Id,    CastKind::IntToInt},
		    {natives.u8Id,    natives.floatId,  CastKind::IntToFP },
		    {natives.u8Id,    natives.doubleId, CastKind::IntToFP },

		    {natives.i16Id,   natives.i32Id,    CastKind::IntToInt},
		    {natives.i16Id,   natives.i64Id,    CastKind::IntToInt},
		    {natives.i16Id,   natives.floatId,  CastKind::IntToFP },
		    {natives.i16Id,   natives.doubleId, CastKind::IntToFP },

		    {natives.u16Id,   natives.i32Id,    CastKind::IntToInt},
		    {natives.u16Id,   natives.u32Id,    CastKind::IntToInt},
		    {natives.u16Id,   natives.i64Id,    CastKind::IntToInt},
		    {natives.u16Id,   natives.u64Id,    CastKind::IntToInt},
		    {natives.u16Id,   natives.floatId,  CastKind::IntToFP },
		    {natives.u16Id,   natives.doubleId, CastKind::IntToFP },

		    {natives.i32Id,   natives.i64Id,    CastKind::IntToInt},
		    {natives.i32Id,   natives.doubleId, CastKind::IntToFP },

		    {natives.u32Id,   natives.i64Id,    CastKind::IntToInt},
		    {natives.u32Id,   natives.u64Id,    CastKind::IntToInt},
		    {natives.u32Id,   natives.doubleId, CastKind::IntToFP },

		    {natives.floatId, natives.doubleId, CastKind::FPToFP  },
		};

		// The narrowing casts, which need an explicit cast node
		const Cast explicitNumericCasts[]{
		    {natives.i8Id,     natives.boolId,   CastKind::IntToInt},
		    {natives.i8Id,     natives.u8Id,     CastKind::IntToInt},
		    {natives.i8Id,     natives.u16Id,    CastKind::IntToInt},
		    {natives.i8Id,     natives.u32Id,    CastKind::IntToInt},
		    {natives.i8Id,     natives.u64Id,    CastKind::IntToInt},

		    {natives.u8Id,     natives.boolId,   CastKind::IntToInt},
		    {natives.u8Id,     natives.i8Id,     CastKind::IntToInt},

		    {natives.i16Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.i16Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.i16Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.i16Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.i16Id,    natives.u32Id,    CastKind::IntToInt},
		    {natives.i16Id,    natives.u64Id,    CastKind::IntToInt},

		    {natives.u16Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.u16Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.u16Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.u16Id,    natives.i16Id,    CastKind::IntToInt},

		    {natives.i32Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.i32Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.i32Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.i32Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.i32Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.i32Id,    natives.u32Id,    CastKind::IntToInt},
		    {natives.i32Id,    natives.u64Id,    CastKind::IntToInt},
		    {natives.i32Id,    natives.floatId,  CastKind::IntToFP },

		    {natives.u32Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.u32Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.u32Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.u32Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.u32Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.u32Id,    natives.i32Id,    CastKind::IntToInt},
		    {natives.u32Id,    natives.floatId,  CastKind::IntToFP },

		    {natives.i64Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.i64Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.i64Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.i64Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.i64Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.i64Id,    natives.i32Id,    CastKind::IntToInt},
		    {natives.i64Id,    natives.u32Id,    CastKind::IntToInt},
		    {natives.i64Id,    natives.u64Id,    CastKind::IntToInt},
		    {natives.i64Id,    natives.floatId,  CastKind::IntToFP },
		    {natives.i64Id,    natives.doubleId, CastKind::IntToFP },

		    {natives.u64Id,    natives.boolId,   CastKind::IntToInt},
		    {natives.u64Id,    natives.i8Id,     CastKind::IntToInt},
		    {natives.u64Id,    natives.u8Id,     CastKind::IntToInt},
		    {natives.u64Id,    natives.i16Id,    CastKind::IntToInt},
		    {natives.u64Id,    natives.u16Id,    CastKind::IntToInt},
		    {natives.u64Id,    natives.i32Id,    CastKind::IntToInt},
		    {natives.u64Id,    natives.u32Id,    CastKind::IntToInt},
		    {natives.u64Id,    natives.i64Id,    CastKind::IntToInt},
		    {natives.u64Id,    natives.floatId,  CastKind::IntToFP },
		    {natives.u64Id,    natives.doubleId, CastKind::IntToFP },

		    {natives.floatId,  natives.boolId,   CastKind::FPToInt },
		    {natives.floatId,  natives.i8Id,     CastKind::FPToInt },
		    {natives.floatId,  natives.u8Id,     CastKind::FPToInt },
		    {natives.floatId,  natives.i16Id,    CastKind::FPToInt },
		    {natives.floatId,  natives.u16Id,    CastKind::FPToInt },
		    {natives.floatId,  natives.i32Id,    CastKind::FPToInt },
		    {natives.floatId,  natives.u32Id,    CastKind::FPToInt },
		    {natives.floatId,  natives.i64Id,    CastKind::FPToInt },
		    {natives.floatId,  natives.u64Id,    CastKind::FPToInt },

		    {natives.doubleId, natives.boolId,   CastKind::FPToInt },
		    {natives.doubleId, natives.i8Id,     CastKind::FPToInt },
		    {natives.doubleId, natives.u8Id,     CastKind::FPToInt },
		    {natives.doubleId, natives.i16Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.u16Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.i32Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.u32Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.i64Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.u64Id,    CastKind::FPToInt },
		    {natives.doubleId, natives.floatId,  CastKind::FPToFP  },
		};

		for (const Cast& cast : implicitNumericCasts)
		{
			RegisterCast(ast, cast, CastFlags::All);
		}
		for (const Cast& cast : explicitNumericCasts)
		{
			RegisterCast(ast, cast, CastFlags::Explicit);
		}
	}

	const Cast* FindImplicitCast(const p::IdContext& ast, Id fromType, Id toType)
	{
		if (const auto* registry = ast.TryGetStatic<SCasts>())
		{
			const Cast type{.fromType = fromType, .toType = toType};
			const p::i32 index = registry->implicitCasts.FindSorted(type);
			if (index != p::NO_INDEX)
			{
				return &registry->implicitCasts[index];
			}
		}
		return nullptr;
	}

	const Cast* FindExplicitCast(const p::IdContext& ast, Id fromType, Id toType)
	{
		if (const auto* registry = ast.TryGetStatic<SCasts>())
		{
			const Cast type{.fromType = fromType, .toType = toType};
			const p::i32 index = registry->explicitCasts.FindSorted(type);
			if (index != p::NO_INDEX)
			{
				return &registry->explicitCasts[index];
			}
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
