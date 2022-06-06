// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Components/CTypeEditor.h"

#include <AST/Components/CExprBinaryOperator.h>
#include <AST/Components/CExprUnaryOperator.h>
#include <AST/Components/CType.h>
#include <AST/Utils/TypeUtils.h>
#include <PECS/Access.h>


namespace rift::Types
{
	void OpenEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id);
	void CloseEditor(TAccessRef<TWrite<CTypeEditor>, CType> access, AST::Id id);
	bool IsEditorOpen(TAccessRef<CTypeEditor> access, AST::Id id);

	constexpr StringView GetUnaryOperatorName(UnaryOperatorType type)
	{
		switch (type)
		{
			case UnaryOperatorType::Not: return "!";
			case UnaryOperatorType::Negation: return "-";
			case UnaryOperatorType::Increment: return "++";
			case UnaryOperatorType::Decrement: return "--";
			case UnaryOperatorType::BitNot: return "~";
		}
		return "";
	}

	constexpr StringView GetUnaryOperatorLongName(UnaryOperatorType type)
	{
		switch (type)
		{
			case UnaryOperatorType::Not: return "not";
			case UnaryOperatorType::Negation: return "negation";
			case UnaryOperatorType::Increment: return "increment";
			case UnaryOperatorType::Decrement: return "decrement";
			case UnaryOperatorType::BitNot: return "bitwise not / one's complement";
		}
		return "";
	}

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
			case BinaryOperatorType::Sub: return "subtract";
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
}    // namespace rift::Types
