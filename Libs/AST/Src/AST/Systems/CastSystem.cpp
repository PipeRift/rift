// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/CastSystem.h"

#include <PipeECS.h>


namespace rift::ast::CastSystem
{
	void RegisterCast(Tree& ast, const Cast& cast, bool implicit)
	{
		auto& registry = ast.GetOrSetStatic<SCasts>();
		registry.explicitCasts.AddSorted(cast);
		if (implicit)
		{
			registry.implicitCasts.AddSorted(cast);
		}
	}

	void RegisterNumericCasts(Tree& ast)
	{
		const NativeTypeIds& natives = ast.GetNativeTypes();

		// Casts that keep every value of the source type, so they happen on their own.
		// The built-in numeric types follow the C++ rules: widening integers and floats,
		// integers that fit a float's mantissa, and bool to any number
		const Cast implicitNumericCasts[]{
		    {natives.boolId,  natives.i8Id    },
		    {natives.boolId,  natives.u8Id    },
		    {natives.boolId,  natives.i16Id   },
		    {natives.boolId,  natives.u16Id   },
		    {natives.boolId,  natives.i32Id   },
		    {natives.boolId,  natives.u32Id   },
		    {natives.boolId,  natives.i64Id   },
		    {natives.boolId,  natives.u64Id   },
		    {natives.boolId,  natives.floatId },
		    {natives.boolId,  natives.doubleId},

		    {natives.i8Id,    natives.i16Id   },
		    {natives.i8Id,    natives.i32Id   },
		    {natives.i8Id,    natives.i64Id   },
		    {natives.i8Id,    natives.floatId },
		    {natives.i8Id,    natives.doubleId},

		    {natives.u8Id,    natives.i16Id   },
		    {natives.u8Id,    natives.u16Id   },
		    {natives.u8Id,    natives.i32Id   },
		    {natives.u8Id,    natives.u32Id   },
		    {natives.u8Id,    natives.i64Id   },
		    {natives.u8Id,    natives.u64Id   },
		    {natives.u8Id,    natives.floatId },
		    {natives.u8Id,    natives.doubleId},

		    {natives.i16Id,   natives.i32Id   },
		    {natives.i16Id,   natives.i64Id   },
		    {natives.i16Id,   natives.floatId },
		    {natives.i16Id,   natives.doubleId},

		    {natives.u16Id,   natives.i32Id   },
		    {natives.u16Id,   natives.u32Id   },
		    {natives.u16Id,   natives.i64Id   },
		    {natives.u16Id,   natives.u64Id   },
		    {natives.u16Id,   natives.floatId },
		    {natives.u16Id,   natives.doubleId},

		    {natives.i32Id,   natives.i64Id   },
		    {natives.i32Id,   natives.doubleId},

		    {natives.u32Id,   natives.i64Id   },
		    {natives.u32Id,   natives.u64Id   },
		    {natives.u32Id,   natives.doubleId},

		    {natives.floatId, natives.doubleId},
		};

		// The narrowing casts, which need an explicit cast node
		const Cast explicitNumericCasts[]{
		    {natives.i8Id,     natives.boolId  },
		    {natives.i8Id,     natives.u8Id    },
		    {natives.i8Id,     natives.u16Id   },
		    {natives.i8Id,     natives.u32Id   },
		    {natives.i8Id,     natives.u64Id   },

		    {natives.u8Id,     natives.boolId  },
		    {natives.u8Id,     natives.i8Id    },

		    {natives.i16Id,    natives.boolId  },
		    {natives.i16Id,    natives.i8Id    },
		    {natives.i16Id,    natives.u8Id    },
		    {natives.i16Id,    natives.u16Id   },
		    {natives.i16Id,    natives.u32Id   },
		    {natives.i16Id,    natives.u64Id   },

		    {natives.u16Id,    natives.boolId  },
		    {natives.u16Id,    natives.i8Id    },
		    {natives.u16Id,    natives.u8Id    },
		    {natives.u16Id,    natives.i16Id   },

		    {natives.i32Id,    natives.boolId  },
		    {natives.i32Id,    natives.i8Id    },
		    {natives.i32Id,    natives.u8Id    },
		    {natives.i32Id,    natives.i16Id   },
		    {natives.i32Id,    natives.u16Id   },
		    {natives.i32Id,    natives.u32Id   },
		    {natives.i32Id,    natives.u64Id   },
		    {natives.i32Id,    natives.floatId },

		    {natives.u32Id,    natives.boolId  },
		    {natives.u32Id,    natives.i8Id    },
		    {natives.u32Id,    natives.u8Id    },
		    {natives.u32Id,    natives.i16Id   },
		    {natives.u32Id,    natives.u16Id   },
		    {natives.u32Id,    natives.i32Id   },
		    {natives.u32Id,    natives.floatId },

		    {natives.i64Id,    natives.boolId  },
		    {natives.i64Id,    natives.i8Id    },
		    {natives.i64Id,    natives.u8Id    },
		    {natives.i64Id,    natives.i16Id   },
		    {natives.i64Id,    natives.u16Id   },
		    {natives.i64Id,    natives.i32Id   },
		    {natives.i64Id,    natives.u32Id   },
		    {natives.i64Id,    natives.u64Id   },
		    {natives.i64Id,    natives.floatId },
		    {natives.i64Id,    natives.doubleId},

		    {natives.u64Id,    natives.boolId  },
		    {natives.u64Id,    natives.i8Id    },
		    {natives.u64Id,    natives.u8Id    },
		    {natives.u64Id,    natives.i16Id   },
		    {natives.u64Id,    natives.u16Id   },
		    {natives.u64Id,    natives.i32Id   },
		    {natives.u64Id,    natives.u32Id   },
		    {natives.u64Id,    natives.i64Id   },
		    {natives.u64Id,    natives.floatId },
		    {natives.u64Id,    natives.doubleId},

		    {natives.floatId,  natives.boolId  },
		    {natives.floatId,  natives.i8Id    },
		    {natives.floatId,  natives.u8Id    },
		    {natives.floatId,  natives.i16Id   },
		    {natives.floatId,  natives.u16Id   },
		    {natives.floatId,  natives.i32Id   },
		    {natives.floatId,  natives.u32Id   },
		    {natives.floatId,  natives.i64Id   },
		    {natives.floatId,  natives.u64Id   },

		    {natives.doubleId, natives.boolId  },
		    {natives.doubleId, natives.i8Id    },
		    {natives.doubleId, natives.u8Id    },
		    {natives.doubleId, natives.i16Id   },
		    {natives.doubleId, natives.u16Id   },
		    {natives.doubleId, natives.i32Id   },
		    {natives.doubleId, natives.u32Id   },
		    {natives.doubleId, natives.i64Id   },
		    {natives.doubleId, natives.u64Id   },
		    {natives.doubleId, natives.floatId },
		};

		for (const Cast& cast : implicitNumericCasts)
		{
			RegisterCast(ast, cast, true);
		}
		for (const Cast& cast : explicitNumericCasts)
		{
			RegisterCast(ast, cast, false);
		}
	}

	const Cast* FindCastImplicit(const p::IdContext& ast, Id fromType, Id toType)
	{
		const auto& casts = ast.GetStatic<SCasts>().implicitCasts;
		const Cast type{.fromType = fromType, .toType = toType};

		const p::i32 index = casts.FindSorted(type);
		return index != p::NO_INDEX ? &casts[index] : nullptr;
	}

	const Cast* FindCast(const p::IdContext& ast, Id fromType, Id toType)
	{
		const auto& casts = ast.GetStatic<SCasts>().explicitCasts;
		const Cast type{.fromType = fromType, .toType = toType};

		const p::i32 index = casts.FindSorted(type);
		return index != p::NO_INDEX ? &casts[index] : nullptr;
	}

	p::TView<const Cast> GetCasts(const p::IdContext& ast)
	{
		return ast.GetStatic<SCasts>().explicitCasts;
	}

	bool CanCastImplicit(p::TIdScopeRef<> scope, Id fromType, Id toType)
	{
		return FindCastImplicit(scope.GetContext(), fromType, toType) != nullptr;
	}

	bool CanCast(p::TIdScopeRef<> scope, Id fromType, Id toType)
	{
		return FindCast(scope.GetContext(), fromType, toType) != nullptr;
	}
}    // namespace rift::ast::CastSystem
