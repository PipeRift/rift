// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "AST/Components/CNamespace.h"

#include <PipeECS.h>
#include <PipeReflect.h>


namespace rift::ast
{
	enum class UnaryOperatorType : p::u8
	{
		Not,          // !
		Negation,     // -
		Increment,    // ++
		Decrement,    // --
		BitNot,       // ~
	};

	enum class BinaryOperatorType : p::u8
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

	enum class TypeMode
	{
		Value,
		Pointer,
		PointerToPointer
	};
}    // namespace rift::ast
P_ENUM(rift::ast::UnaryOperatorType)
P_ENUM(rift::ast::BinaryOperatorType)
P_ENUM(rift::ast::TypeMode)


namespace rift::ast
{
	struct CExpression
	{
		P_STRUCT(CExpression)
	};


	struct CExprCall : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprCall)

		P_PROP(function)
		Namespace function;
	};


	// Data pointing to the id of the function from CExprCall's type and function names
	struct CExprCallId : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprCallId, p::TF_NotSerialized)

		// Id pointing to the function declaration
		P_PROP(functionId)
		p::Id functionId = p::NoId;


		CExprCallId(p::Id functionId = p::NoId) : functionId{functionId} {}
	};


	struct CExprUnaryOperator : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprUnaryOperator)

		P_PROP(type)
		UnaryOperatorType type = UnaryOperatorType::Not;


		CExprUnaryOperator() = default;
		CExprUnaryOperator(UnaryOperatorType type) : type{type} {}
	};


	struct CExprBinaryOperator : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprBinaryOperator)

		P_PROP(type)
		BinaryOperatorType type = BinaryOperatorType::Add;


		CExprBinaryOperator() = default;
		CExprBinaryOperator(BinaryOperatorType type) : type{type} {}
	};


	struct CExprDeclRef : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprDeclRef)

		P_PROP(ownerName)
		p::Tag ownerName;

		P_PROP(name)
		p::Tag name;
	};


	struct CExprDeclRefId : public CExpression
	{
		using Super = CExpression;
		P_STRUCT(CExprDeclRefId)

		P_PROP(declarationId)
		p::Id declarationId = p::NoId;
	};


	struct ExprInput
	{
		P_STRUCT(ExprInput)

		P_PROP(nodeId)
		p::Id nodeId = p::NoId;

		P_PROP(pinId)
		p::Id pinId = p::NoId;


		ExprInput() = default;

		bool IsNone() const
		{
			return p::IsNone(nodeId) || p::IsNone(pinId);
		}
	};


	struct ExprOutput
	{
		P_STRUCT(ExprOutput)

		P_PROP(nodeId)
		p::Id nodeId = p::NoId;

		P_PROP(pinId)
		p::Id pinId = p::NoId;


		ExprOutput() = default;

		bool IsNone() const
		{
			return p::IsNone(nodeId) || p::IsNone(pinId);
		}
	};


	struct CExprInputs
	{
		P_STRUCT(CExprInputs)

		P_PROP(linkedOutputs)
		p::TArray<ExprOutput> linkedOutputs;

		P_PROP(pinIds)
		p::TArray<p::Id> pinIds;


		CExprInputs& Add(p::Id pinId)
		{
			linkedOutputs.Add();
			pinIds.Add(pinId);
			return *this;
		}

		CExprInputs& Insert(p::i32 index, p::Id pinId)
		{
			pinIds.Insert(index, pinId);
			linkedOutputs.Insert(index);
			return *this;
		}

		CExprInputs& Swap(p::i32 firstIndex, p::i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			linkedOutputs.Swap(firstIndex, secondIndex);
			return *this;
		}

		void Resize(p::i32 count)
		{
			linkedOutputs.Resize(count);
			pinIds.Resize(count, p::NoId);
		}
	};


	struct CExprOutputs
	{
		P_STRUCT(CExprOutputs)

		P_PROP(pinIds)
		p::TArray<p::Id> pinIds;


		CExprOutputs() {}
		CExprOutputs(p::Id pinId)
		{
			Add(pinId);
		}

		CExprOutputs& Add(p::Id pinId)
		{
			pinIds.Add(pinId);
			return *this;
		}

		CExprOutputs& Insert(p::i32 index, p::Id pinId)
		{
			pinIds.Insert(index, pinId);
			return *this;
		}

		CExprOutputs& Swap(p::i32 firstIndex, p::i32 secondIndex)
		{
			pinIds.Swap(firstIndex, secondIndex);
			return *this;
		}
	};


	struct CExprType
	{
		P_STRUCT(CExprType)

		P_PROP(type)
		Namespace type;

		P_PROP(mode)
		TypeMode mode = TypeMode::Value;
	};


	struct CExprTypeId
	{
		P_STRUCT(CExprTypeId)

		P_PROP(id, p::PF_NotSerialized)
		p::Id id = p::NoId;

		P_PROP(mode)
		TypeMode mode = TypeMode::Value;
	};


	static void Read(p::Reader& ct, CExprCall& val)
	{
		ct.Serialize(val.function);
	}
	static void Write(p::Writer& ct, const CExprCall& val)
	{
		ct.Serialize(val.function);
	}

	static void Read(p::Reader& ct, CExprType& val)
	{
		ct.Serialize(val.type);
	}
	static void Write(p::Writer& ct, const CExprType& val)
	{
		ct.Serialize(val.type);
	}
}    // namespace rift::ast
