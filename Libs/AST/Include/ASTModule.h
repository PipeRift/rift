// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include <Module.h>


namespace rift
{
	namespace ast
	{
		class Tree;
	}

	class ASTModule : public Module
	{
	public:
		using Super = Module;
		P_CLASS(ASTModule)

	public:
		static const p::Tag structType;
		static const p::Tag classType;
		static const p::Tag staticType;


		void Load() override;
	};
}    // namespace rift
