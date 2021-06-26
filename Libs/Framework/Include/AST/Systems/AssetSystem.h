// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <CoreObject.h>


namespace Rift
{
	namespace AST
	{
		struct Tree;
	}

	class AssetSystem : public Object
	{
		CLASS(AssetSystem, Object)

	public:
		void Init(AST::Tree& ast);
		void Tick(AST::Tree& ast);
	};
}    // namespace Rift
