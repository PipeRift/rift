// Copyright 2015-2023 Piperift - All rights reserved

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
		P_CLASS(ASTModule, Module)

	public:
		static const p::Tag structType;
		static const p::Tag classType;
		static const p::Tag staticType;


		void Load() override;
	};
}    // namespace rift
