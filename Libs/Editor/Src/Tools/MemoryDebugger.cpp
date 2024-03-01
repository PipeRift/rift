// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/MemoryDebugger.h"

#include <Pipe/Core/String.h>
#include <Pipe/Memory/Alloc.h>
#include <Pipe/Memory/MemoryStats.h>
#include <PipeMath.h>
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
			auto* stats = GetHeapStats();
			UI::Text(Strings::Format("Used: {}", Strings::ParseMemorySize(stats->used)));

			if (UI::BeginChild("Allocations"))
			{
				const i32 shown = p::Min<i32>(10000, i32(stats->allocations.Size()));
				for (i32 i = 0; i < shown; ++i)
				{
					const auto& allocation = stats->allocations[i];
					label.clear();
					Strings::FormatTo(label, "{}", allocation.ptr);
					if (UI::TreeNodeEx(label.c_str()))
					{
						label.clear();
						Strings::FormatTo(label, "Address: {}, Size: {}", (void*)allocation.ptr,
						    Strings::ParseMemorySize(allocation.size));
						UI::Text(label.c_str());

#if PIPE_ENABLE_ALLOCATION_STACKS
						UI::Text("Stack trace:");
						const auto& stack = stats->allocationStacks[i];
						backward::TraceResolver tr;
						tr.load_stacktrace(stack);
						for (sizet i = 0; i < stack.size(); ++i)
						{
							backward::ResolvedTrace trace = tr.resolve(stack[i]);
							label.clear();
							Strings::FormatTo(label, "#{} {} {} [{}]", i, trace.object_filename,
							    trace.object_function, trace.addr);
							UI::Text(label);
						}
#endif
						UI::TreePop();
					}
				}
			}
			UI::EndChild();
		}
		UI::End();
	}
}    // namespace rift::editor
