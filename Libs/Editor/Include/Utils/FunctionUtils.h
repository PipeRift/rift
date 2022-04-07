// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Components/CExprBinaryOperator.h>
#include <AST/Utils/FunctionUtils.h>


namespace Rift::Functions
{
	constexpr StringView GetBinaryOperatorName(BinaryOperatorType type)
	{
		switch (type)
		{
			case BinaryOperatorType::Add: return "+";
			case BinaryOperatorType::Sub: return "-";
			case BinaryOperatorType::Mul: return "*";
			case BinaryOperatorType::Div: return "/";
			case BinaryOperatorType::Mod: return "%";

			case BinaryOperatorType::Equal: return "==";
			case BinaryOperatorType::NotEqual: return "!=";
			case BinaryOperatorType::Greater: return ">";
			case BinaryOperatorType::Less: return "<";
			case BinaryOperatorType::GreaterOrEqual: return ">=";
			case BinaryOperatorType::LessOrEqual: return "<=";

			case BinaryOperatorType::And: return "&&";
			case BinaryOperatorType::Or: return "||";
			case BinaryOperatorType::BitAnd: return "&";
			case BinaryOperatorType::BitOr: return "|";
			case BinaryOperatorType::Xor: return "^";
		}
		return "";
	}

	constexpr StringView GetBinaryOperatorLongName(BinaryOperatorType type)
	{
		switch (type)
		{
			case BinaryOperatorType::Add: return "add";
			case BinaryOperatorType::Sub: return "substract";
			case BinaryOperatorType::Mul: return "multiply";
			case BinaryOperatorType::Div: return "divide";
			case BinaryOperatorType::Mod: return "module";

			case BinaryOperatorType::Equal: return "equal";
			case BinaryOperatorType::NotEqual: return "not equal";
			case BinaryOperatorType::Greater: return "greater";
			case BinaryOperatorType::Less: return "less";
			case BinaryOperatorType::GreaterOrEqual: return "greater or equal";
			case BinaryOperatorType::LessOrEqual: return "less or equal";

			case BinaryOperatorType::And: return "and";
			case BinaryOperatorType::Or: return "or";
			case BinaryOperatorType::BitAnd: return "bitwise and";
			case BinaryOperatorType::BitOr: return "bitwise or";
			case BinaryOperatorType::Xor: return "exclusive or";
		}
		return "";
	}
}    // namespace Rift::Functions
