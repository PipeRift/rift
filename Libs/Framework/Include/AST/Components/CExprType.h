// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	enum class TypeMode
	{
		Value,
		Pointer,
		PointerToPointer
	};
}    // namespace rift::AST
ENUM(rift::AST::TypeMode)


namespace rift::AST
{
	struct CExprType : public p::Struct
	{
		STRUCT(CExprType, p::Struct)

		PROP(type)
		AST::Namespace type;

		PROP(mode)
		TypeMode mode = TypeMode::Value;
	};

	static void Read(Reader& ct, CExprType& val)
	{
		ct.Serialize(val.type);
	}
	static void Write(Writer& ct, const CExprType& val)
	{
		ct.Serialize(val.type);
	}


	struct CExprTypeId : public p::Struct
	{
		STRUCT(CExprTypeId, p::Struct)

		PROP(id, p::Prop_NotSerialized)
		AST::Id id = AST::NoId;

		PROP(mode)
		TypeMode mode = TypeMode::Value;
	};

}    // namespace rift::AST
