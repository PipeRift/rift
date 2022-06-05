// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <AST/Components/CExpression.h>


namespace rift
{
	enum class UnaryOperatorType : u8
	{
		Not,          // !
		Negation,     // -
		Increment,    // ++
		Decrement,    // --
		BitNot,       // ~
	};
}    // namespace rift
ENUM(rift::UnaryOperatorType)


namespace rift
{
	struct CExprUnaryOperator : public CExpression
	{
		STRUCT(CExprUnaryOperator, CExpression)

		PROP(type)
		UnaryOperatorType type = UnaryOperatorType::Not;


		CExprUnaryOperator() = default;
		CExprUnaryOperator(UnaryOperatorType type) : type{type} {}
	};
}    // namespace rift
