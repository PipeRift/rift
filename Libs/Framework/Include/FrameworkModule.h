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
		static const p::Name structType;
		static const p::Name classType;
		static const p::Name staticType;

		p::TArray<FileTypeDescriptor> fileTypes;


		FrameworkModule();
	};
}    // namespace rift
