// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Components/CTypeEditor.h"

#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Utils/TypeUtils.h>
#include <PipeECS.h>


namespace rift::Editor
{
	void OpenType(TAccessRef<TWrite<CTypeEditor>, ast::CDeclType> access, ast::Id id);
	void CloseType(TAccessRef<TWrite<CTypeEditor>, ast::CDeclType> access, ast::Id id);
	bool IsTypeOpen(TAccessRef<CTypeEditor> access, ast::Id id);

	constexpr StringView GetUnaryOperatorName(ast::UnaryOperatorType type)
	{
		switch (type)
		{
			case ast::UnaryOperatorType::Not: return "!";
			case ast::UnaryOperatorType::Negation: return "-";
			case ast::UnaryOperatorType::Increment: return "++";
			case ast::UnaryOperatorType::Decrement: return "--";
			case ast::UnaryOperatorType::BitNot: return "~";
		}
		return "";
	}

	constexpr StringView GetUnaryOperatorLongName(ast::UnaryOperatorType type)
	{
		switch (type)
		{
			case ast::UnaryOperatorType::Not: return "not";
			case ast::UnaryOperatorType::Negation: return "negation";
			case ast::UnaryOperatorType::Increment: return "increment";
			case ast::UnaryOperatorType::Decrement: return "decrement";
			case ast::UnaryOperatorType::BitNot: return "bitwise not / one's complement";
		}
		return "";
	}

	constexpr StringView GetBinaryOperatorName(ast::BinaryOperatorType type)
	{
		switch (type)
		{
			case ast::BinaryOperatorType::Add: return "+";
			case ast::BinaryOperatorType::Sub: return "-";
			case ast::BinaryOperatorType::Mul: return "*";
			case ast::BinaryOperatorType::Div: return "/";
			case ast::BinaryOperatorType::Mod: return "%";

			case ast::BinaryOperatorType::Equal: return "==";
			case ast::BinaryOperatorType::NotEqual: return "!=";
			case ast::BinaryOperatorType::Greater: return ">";
			case ast::BinaryOperatorType::Less: return "<";
			case ast::BinaryOperatorType::GreaterOrEqual: return ">=";
			case ast::BinaryOperatorType::LessOrEqual: return "<=";

			case ast::BinaryOperatorType::And: return "&&";
			case ast::BinaryOperatorType::Or: return "||";
			case ast::BinaryOperatorType::BitAnd: return "&";
			case ast::BinaryOperatorType::BitOr: return "|";
			case ast::BinaryOperatorType::Xor: return "^";
		}
		return "";
	}

	constexpr StringView GetBinaryOperatorLongName(ast::BinaryOperatorType type)
	{
		switch (type)
		{
			case ast::BinaryOperatorType::Add: return "add";
			case ast::BinaryOperatorType::Sub: return "subtract";
			case ast::BinaryOperatorType::Mul: return "multiply";
			case ast::BinaryOperatorType::Div: return "divide";
			case ast::BinaryOperatorType::Mod: return "module";

			case ast::BinaryOperatorType::Equal: return "equal";
			case ast::BinaryOperatorType::NotEqual: return "not equal";
			case ast::BinaryOperatorType::Greater: return "greater";
			case ast::BinaryOperatorType::Less: return "less";
			case ast::BinaryOperatorType::GreaterOrEqual: return "greater or equal";
			case ast::BinaryOperatorType::LessOrEqual: return "less or equal";

			case ast::BinaryOperatorType::And: return "and";
			case ast::BinaryOperatorType::Or: return "or";
			case ast::BinaryOperatorType::BitAnd: return "bitwise and";
			case ast::BinaryOperatorType::BitOr: return "bitwise or";
			case ast::BinaryOperatorType::Xor: return "exclusive or";
		}
		return "";
	}
}    // namespace rift::Editor
