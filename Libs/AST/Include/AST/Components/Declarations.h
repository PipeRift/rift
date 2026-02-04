// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once


#include <PipeECS.h>
#include <PipeReflect.h>


namespace rift::ast
{
	struct CDeclStatic
	{
		P_STRUCT(CDeclStatic)
	};


	struct CDeclRecord
	{
		P_STRUCT(CDeclRecord)
	};


	struct CDeclStruct : public CDeclRecord
	{
		using Super = CDeclRecord;
		P_STRUCT(CDeclStruct)
	};


	struct CDeclClass : public CDeclRecord
	{
		using Super = CDeclRecord;
		P_STRUCT(CDeclClass)
	};


	struct CDeclType
	{
		P_STRUCT(CDeclType)

		P_PROP(typeId)
		p::Tag typeId;
	};


	struct CDeclNative : public CDeclRecord
	{
		using Super = CDeclRecord;
		P_STRUCT(CDeclNative)
	};


	struct CDeclFunction
	{
		P_STRUCT(CDeclFunction)
	};


	struct CDeclVariable
	{
		P_STRUCT(CDeclVariable)

		P_PROP(typeId, p::PF_NotSerialized)
		p::Id typeId = p::NoId;
	};
}    // namespace rift::ast
