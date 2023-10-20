// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Components/CTypeEditor.h"

#include <AST/Components/CDeclType.h>
#include <AST/Components/CExprBinaryOperator.h>
#include <AST/Components/CExprUnaryOperator.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeECS.h>


namespace rift::Editor
{
	void OpenType(TAccessRef<TWrite<CTypeEditor>, AST::CDeclType> access, AST::Id id);
	void CloseType(TAccessRef<TWrite<CTypeEditor>, AST::CDeclType> access, AST::Id id);
	bool IsTypeOpen(TAccessRef<CTypeEditor> access, AST::Id id);

	constexpr StringView GetUnaryOperatorName(AST::UnaryOperatorType type)
	{
		switch (type)
		{
			case AST::UnaryOperatorType::Not: return "!";
			case AST::UnaryOperatorType::Negation: return "-";
			case AST::UnaryOperatorType::Increment: return "++";
			case AST::UnaryOperatorType::Decrement: return "--";
			case AST::UnaryOperatorType::BitNot: return "~";
		}
		return "";
	}

	constexpr StringView GetUnaryOperatorLongName(AST::UnaryOperatorType type)
	{
		switch (type)
		{
			case AST::UnaryOperatorType::Not: return "not";
			case AST::UnaryOperatorType::Negation: return "negation";
			case AST::UnaryOperatorType::Increment: return "increment";
			case AST::UnaryOperatorType::Decrement: return "decrement";
			case AST::UnaryOperatorType::BitNot: return "bitwise not / one's complement";
		}
		return "";
	}

	constexpr StringView GetBinaryOperatorName(AST::BinaryOperatorType type)
	{
		switch (type)
		{
			case AST::BinaryOperatorType::Add: return "+";
			case AST::BinaryOperatorType::Sub: return "-";
			case AST::BinaryOperatorType::Mul: return "*";
			case AST::BinaryOperatorType::Div: return "/";
			case AST::BinaryOperatorType::Mod: return "%";

			case AST::BinaryOperatorType::Equal: return "==";
			case AST::BinaryOperatorType::NotEqual: return "!=";
			case AST::BinaryOperatorType::Greater: return ">";
			case AST::BinaryOperatorType::Less: return "<";
			case AST::BinaryOperatorType::GreaterOrEqual: return ">=";
			case AST::BinaryOperatorType::LessOrEqual: return "<=";

			case AST::BinaryOperatorType::And: return "&&";
			case AST::BinaryOperatorType::Or: return "||";
			case AST::BinaryOperatorType::BitAnd: return "&";
			case AST::BinaryOperatorType::BitOr: return "|";
			case AST::BinaryOperatorType::Xor: return "^";
		}
		return "";
	}

	constexpr StringView GetBinaryOperatorLongName(AST::BinaryOperatorType type)
	{
		switch (type)
		{
			case AST::BinaryOperatorType::Add: return "add";
			case AST::BinaryOperatorType::Sub: return "subtract";
			case AST::BinaryOperatorType::Mul: return "multiply";
			case AST::BinaryOperatorType::Div: return "divide";
			case AST::BinaryOperatorType::Mod: return "module";

			case AST::BinaryOperatorType::Equal: return "equal";
			case AST::BinaryOperatorType::NotEqual: return "not equal";
			case AST::BinaryOperatorType::Greater: return "greater";
			case AST::BinaryOperatorType::Less: return "less";
			case AST::BinaryOperatorType::GreaterOrEqual: return "greater or equal";
			case AST::BinaryOperatorType::LessOrEqual: return "less or equal";

			case AST::BinaryOperatorType::And: return "and";
			case AST::BinaryOperatorType::Or: return "or";
			case AST::BinaryOperatorType::BitAnd: return "bitwise and";
			case AST::BinaryOperatorType::BitOr: return "bitwise or";
			case AST::BinaryOperatorType::Xor: return "exclusive or";
		}
		return "";
	}
}    // namespace rift::Editor
