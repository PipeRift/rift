// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <AST/Components/CExpression.h>


namespace rift::AST
{
	enum class UnaryOperatorType : p::u8
	{
		Not,          // !
		Negation,     // -
		Increment,    // ++
		Decrement,    // --
		BitNot,       // ~
	};
}    // namespace rift::AST
ENUM(rift::AST::UnaryOperatorType)


namespace rift::AST
{
	struct CExprUnaryOperator : public CExpression
	{
		P_STRUCT(CExprUnaryOperator, CExpression)

		P_PROP(type)
		UnaryOperatorType type = UnaryOperatorType::Not;


		CExprUnaryOperator() = default;
		CExprUnaryOperator(UnaryOperatorType type) : type{type} {}
	};
}    // namespace rift::AST
