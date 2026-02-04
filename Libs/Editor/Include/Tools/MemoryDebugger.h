// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <Pipe/Core/StringView.h>
#include <PipeMemory.h>
#include <PipePlatform.h>
#include <PipeVectors.h>


namespace rift::editor
{
	using namespace p;


	struct MemoryDebugger
	{
		bool open = false;


		MemoryDebugger();
		void Draw();
	};
}    // namespace rift::editor
