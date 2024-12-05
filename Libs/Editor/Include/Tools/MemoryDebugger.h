// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/StringView.h>
#include <Pipe/Memory/Block.h>
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
