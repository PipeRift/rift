// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct CDeclStatic : public p::Struct
	{
		P_STRUCT(CDeclStatic, p::Struct)
	};


	struct CDeclRecord : public p::Struct
	{
		P_STRUCT(CDeclRecord, p::Struct)
	};


	struct CDeclStruct : public CDeclRecord
	{
		P_STRUCT(CDeclStruct, CDeclRecord, )
	};


	struct CDeclClass : public CDeclRecord
	{
		P_STRUCT(CDeclClass, CDeclRecord)
	};


	struct CDeclType : public p::Struct
	{
		P_STRUCT(CDeclType, p::Struct)

		P_PROP(typeId)
		p::Tag typeId;
	};


	struct CDeclNative : public CDeclRecord
	{
		P_STRUCT(CDeclNative, CDeclRecord)
	};


	struct CDeclFunction : public p::Struct
	{
		P_STRUCT(CDeclFunction, p::Struct)
	};


	struct CDeclVariable : public p::Struct
	{
		P_STRUCT(CDeclVariable, p::Struct)

		P_PROP(typeId, p::Prop_NotSerialized)
		p::Id typeId = p::NoId;
	};
}    // namespace rift::ast
