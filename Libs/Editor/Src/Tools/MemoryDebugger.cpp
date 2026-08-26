// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Tools/MemoryDebugger.h"

#include <Pipe/Core/String.h>
#include <Pipe/Memory/MemoryStats.h>
#include <PipeMath.h>
#include <PipeMemoryArenas.h>
#include <UI/UI.h>
#include <UI/UIImgui.h>


// External
#include <imgui_internal.h>


namespace rift::editor
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
			String label;
			auto* stats = GetHeapArena().GetStats();
			stats->CollectStats();
			UI::Text(Strings::Format("Used: {}", Strings::ParseMemorySize(stats->used)));

			if (UI::BeginChild("Allocations"))
			{
				const i32 shown = p::Min<i32>(10000, i32(stats->events.Size()));
				for (i32 i = 0; i < shown; ++i)
				{
					if (!stats->live[i])
					{
						continue;    // Event is not a live allocation
					}
					const auto& event = stats->events[i];
					label.clear();
					Strings::FormatTo(label, "{}", (void*)event.GetPtr());
					if (UI::TreeNodeEx(label.c_str()))
					{
						label.clear();
						Strings::FormatTo(label, "Address: {}, Size: {}", (void*)event.GetPtr(),
						    Strings::ParseMemorySize(event.GetSize()));
						UI::Text(label.c_str());
						UI::TreePop();
					}
				}
			}
			UI::EndChild();
		}
		UI::End();
	}
}    // namespace rift::editor
