// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <AST/Components/CExpression.h>


namespace rift::AST
{
	enum class BinaryOperatorType : u8
	{
		// Mathematic
		Add,    // +
		Sub,    // -
		Mul,    // *
		Div,    // /
		Mod,    // %

		// Comparison
		Equal,             // ==
		NotEqual,          // !=
		Greater,           // >
		Less,              // <
		GreaterOrEqual,    // >=
		LessOrEqual,       // <=

		// Logical
		And,       // &&
		Or,        // ||
		BitAnd,    // &
		BitOr,     // |
		Xor        // ^
	};
}    // namespace rift::AST
ENUM(rift::AST::BinaryOperatorType)


namespace rift::AST
{
	struct CExprBinaryOperator : public CExpression
	{
		P_STRUCT(CExprBinaryOperator, CExpression)

		P_PROP(type)
		BinaryOperatorType type = BinaryOperatorType::Add;


		CExprBinaryOperator() = default;
		CExprBinaryOperator(BinaryOperatorType type) : type{type} {}
	};
}    // namespace rift::AST
