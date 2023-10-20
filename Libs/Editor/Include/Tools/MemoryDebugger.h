// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Platform.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/Memory/BigBestFitArena.h>
#include <Pipe/Memory/Block.h>
#include <PipeVectors.h>



namespace rift::Editor
{
	using namespace p;


	struct MemoryDebugger
	{
		bool open = false;


		MemoryDebugger();
		void Draw();
	};
}    // namespace rift::Editor
