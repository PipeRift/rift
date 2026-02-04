// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once


#include <PipeECS.h>
#include <PipeReflect.h>


namespace rift::ast
{
	struct CStmtFor
	{
		P_STRUCT(CStmtFor)
	};


	struct CStmtIf
	{
		P_STRUCT(CStmtIf)
	};


	struct CStmtInput
	{
		P_STRUCT(CStmtInput)

		P_PROP(linkOutputNode)
		p::Id linkOutputNode = p::NoId;
	};


	struct CStmtOutput
	{
		P_STRUCT(CStmtOutput)

		P_PROP(linkInputNode)
		p::Id linkInputNode = p::NoId;
	};


	struct CStmtOutputs
	{
		P_STRUCT(CStmtOutputs)

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
	struct CStmtReturn
	{
		P_STRUCT(CStmtReturn)
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
