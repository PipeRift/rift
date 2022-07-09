// Copyright 2015-2022 Piperift - All rights reserved

#include "Tools/MemoryDebugger.h"

#include <Pipe/Core/String.h>
#include <Pipe/Math/Math.h>
#include <Pipe/Memory/Alloc.h>
#include <Pipe/Memory/MemoryStats.h>
#include <UI/UI.h>


// External
#include <imgui_internal.h>


namespace rift
{
	static constexpr Color gFreeColor{210, 56, 41};    // Red
	static constexpr Color gUsedColor{56, 210, 41};    // Green


	MemoryDebugger::MemoryDebugger() {}

	void MemoryDebugger::Draw()
	{
		if (!open)
		{
			return;
		}

		if (UI::Begin("Memory", &open))
		{
			auto* stats = GetHeapStats();
			UI::Text(Strings::Format("Used: {}", Strings::ParseMemorySize(stats->used)));
		}
		UI::End();
	}
}    // namespace rift
