// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct CStmtFor : public p::Struct
	{
		P_STRUCT(CStmtFor, p::Struct)
	};


	struct CStmtIf : public p::Struct
	{
		P_STRUCT(CStmtIf, p::Struct)
	};


	struct CStmtInput : public p::Struct
	{
		P_STRUCT(CStmtInput, p::Struct)

		P_PROP(linkOutputNode)
		p::Id linkOutputNode = p::NoId;
	};


	struct CStmtOutput : public p::Struct
	{
		P_STRUCT(CStmtOutput, p::Struct)

		P_PROP(linkInputNode)
		p::Id linkInputNode = p::NoId;
	};


	struct CStmtOutputs : public p::Struct
	{
		P_STRUCT(CStmtOutputs, p::Struct)

		// Both arrays keep the same index to the input node and the output pin
		P_PROP(pinIds)
		p::TArray<p::Id> pinIds;
		P_PROP(linkInputNodes)
		p::TArray<p::Id> linkInputNodes;


		CStmtOutputs() = default;
		CStmtOutputs(p::TArray<p::Id> pins)
		    : pinIds{Move(pins)}, linkInputNodes(pinIds.Size(), p::NoId)
		{}
	};

	/** Represents a return expression of a function
	 * Return arguments are dynamically populated depending on the function this expression is
	 * connected to.
	 */
	struct CStmtReturn : public p::Struct
	{
		P_STRUCT(CStmtReturn, p::Struct)
	};


	static void Read(p::Reader& ct, CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Write(p::Writer& ct, const CStmtInput& val)
	{
		ct.Serialize(val.linkOutputNode);
	}
	static void Read(p::Reader& ct, CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
	static void Write(p::Writer& ct, const CStmtOutput& val)
	{
		ct.Serialize(val.linkInputNode);
	}
}    // namespace rift::ast
