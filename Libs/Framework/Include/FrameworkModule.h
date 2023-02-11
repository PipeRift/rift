// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Module.h>


namespace rift
{
	namespace AST
	{
		class Tree;
	}

	class FrameworkModule : public Module
	{
		CLASS(FrameworkModule, Module)

	public:
		static const p::Tag structType;
		static const p::Tag classType;
		static const p::Tag staticType;


		void Load() override;
	};
}    // namespace rift
