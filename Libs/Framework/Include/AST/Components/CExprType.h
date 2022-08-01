// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Reflect/Struct.h>


namespace rift
{
	enum class CExprTypePointer
	{
		NotPointer,
		Pointer,
		PointerToPointer
	};
}    // namespace rift
ENUM(rift::CExprTypePointer)


namespace rift
{
	struct CExprType : public p::Struct
	{
		STRUCT(CExprType, p::Struct)

		PROP(type)
		AST::Namespace type;

		PROP(pointer)
		CExprTypePointer pointer = CExprTypePointer::NotPointer;
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

		PROP(pointer)
		CExprTypePointer pointer = CExprTypePointer::NotPointer;


		CExprTypeId(AST::Id id = AST::NoId) : id{id} {}
	};

}    // namespace rift
