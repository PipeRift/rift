// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include <ASTModule.h>
#include <Compiler/Backend.h>
#include <Module.h>
#include <Pipe/Core/EnumFlags.h>
#include <View.h>


namespace rift
{
	class GraphViewModule : public Module
	{
	public:
		using Super = Module;
		P_CLASS(GraphViewModule)

	public:
		void Load() override;

		static void DrawEditor() {}
	};
}    // namespace rift
