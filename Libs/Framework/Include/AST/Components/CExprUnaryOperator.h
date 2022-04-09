// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Types.h"

#include <AST/Components/CExpression.h>


namespace Rift
{
	enum class UnaryOperatorType : u8
	{
		Not,          // !
		Negation,     // -
		Increment,    // ++
		Decrement,    // --
		BitNot,       // ~
	};
	ENUM(UnaryOperatorType)


	struct CExprUnaryOperator : public CExpression
	{
		STRUCT(CExprUnaryOperator, CExpression)

		PROP(type)
		UnaryOperatorType type = UnaryOperatorType::Not;


		CExprUnaryOperator() = default;
		CExprUnaryOperator(UnaryOperatorType type) : type{type} {}
	};
}    // namespace Rift
