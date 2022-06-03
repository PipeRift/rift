// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <AST/Components/CExpression.h>


namespace Rift
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
	ENUM(BinaryOperatorType)


	struct CExprBinaryOperator : public CExpression
	{
		STRUCT(CExprBinaryOperator, CExpression)

		PROP(type)
		BinaryOperatorType type = BinaryOperatorType::Add;


		CExprBinaryOperator() = default;
		CExprBinaryOperator(BinaryOperatorType type) : type{type} {}
	};
}    // namespace Rift
