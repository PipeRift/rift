// Copyright 2015-2023 Piperift - All rights reserved

#include "Tools/BigBestFitArenaDebugger.h"

#include <Pipe/Core/String.h>
#include <Pipe/Memory/Alloc.h>
#include <PipeMath.h>
#include <UI/UI.h>


// External
#include <imgui_internal.h>


namespace rift::editor
{
	static constexpr Color gFreeColor{210, 56, 41};    // Red
	static constexpr Color gUsedColor{56, 210, 41};    // Green


	void DrawMemoryRect(ImGuiWindow* window, const MemoryGrid& grid, ImRect box, v2_u32 min,
	    v2_u32 max, const Color& color)
	{
		const v2 fmin = v2{box.Min} + v2{min} * grid.unitSize;
		const v2 fmax = v2{box.Min} + (v2{max} * grid.unitSize) + grid.unitSize;

		window->DrawList->AddRectFilled(ImVec2(fmin), ImVec2(fmax), color.ToPackedABGR());
	}

	i32 DrawMemoryBlock(StringView label, MemoryGrid& grid,
	    const TArray<BigBestFitArena::Slot>& freeSlots, v2 graphSize = v2::Zero())
	{
		ImGuiContext& g     = *GImGui;
		ImGuiWindow* window = UI::GetCurrentWindow();
		if (window->SkipItems)
		{
			return -1;
		}

		const ImGuiStyle& style = g.Style;
		const ImGuiID id        = window->GetID(label.data());
		const ImVec2 labelSize  = UI::CalcTextSize(label.data(), nullptr, true);

		if (graphSize.x == 0.0f)
		{
			graphSize.x = UI::CalcItemWidth();
		}
		if (graphSize.y == 0.0f)
		{
			graphSize.y = labelSize.y;
		}

		grid.UpdateGridScale(graphSize.x);
		graphSize.y = p::Max(graphSize.y, grid.GetHeight());

		const ImRect frameBox(window->DC.CursorPos, ImVec2(v2(window->DC.CursorPos) + graphSize));

		const ImRect totalBox(ImVec2(frameBox.Min),
		    ImVec2(v2(frameBox.Max.x, frameBox.Max.y)
		           + v2(labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f, 0)));

		UI::ItemSize(totalBox, style.FramePadding.y);
		if (!UI::ItemAdd(totalBox, 0, &frameBox))
		{
			return -1;
		}
		const bool hovered = UI::ItemHoverable(frameBox, id, ImGuiItemFlags_None);

		UI::RenderFrame(frameBox.Min, frameBox.Max, UI::GetColorU32(ImGuiCol_FrameBg), true,
		    style.FrameRounding);

		// Render
		{
			DrawMemoryRect(window, grid, frameBox, {0, 0}, {grid.numColumns - 1, grid.numRows - 2},
			    gUsedColor);
			DrawMemoryRect(window, grid, frameBox, {0, grid.numRows - 1},
			    {grid.GetX(grid.block->size), grid.numRows - 1}, gUsedColor);

			DrawMemoryRect(window, grid, frameBox, {0, 0}, {0, 0}, gFreeColor);

			for (auto& slot : freeSlots)
			{
				const sizet startOffset = sizet(slot.offset);
				const sizet endOffset   = sizet(slot.offset + slot.size);
				const u32 startX        = grid.GetX(startOffset);
				const u32 endX          = grid.GetX(endOffset);
				const u32 startY        = grid.GetY(startOffset);
				const u32 endY          = p::Min(grid.GetY(endOffset), grid.numRows - 1);

				// Draw incomplete rows
				if (startY != endY)
				{
					DrawMemoryRect(window, grid, frameBox, {startX, startY},
					    {grid.numColumns - 1, startY}, gFreeColor);

					DrawMemoryRect(window, grid, frameBox, {0, endY}, {endX, endY}, gFreeColor);

					// Draw full rows
					if (endY - startY > 1)
					{
						DrawMemoryRect(window, grid, frameBox, {0, startY + 1},
						    {grid.numColumns - 1, endY - 1}, gFreeColor);
					}
				}
				else
				{
					DrawMemoryRect(
					    window, grid, frameBox, {startX, endY}, {endX, endY}, gFreeColor);
				}
			}
		}

		if (labelSize.x > 0.0f)
		{
			UI::RenderText(
			    ImVec2(frameBox.Max.x + style.ItemInnerSpacing.x, frameBox.Min.y), label.data());
		}

		// Return hovered index or -1 if none are hovered.
		// This is currently not exposed in the public API because we need a larger redesign of the
		// whole thing, but in the short-term we are making it available in PlotEx().
		// return idx_hovered;
		return -1;
	}


	void MemoryGrid::UpdateGridScale(float availableWidth)
	{
		numColumns  = u32(availableWidth / unitSize.x);
		bytesPerRow = memoryScale * numColumns;
		numRows     = u32(block->size / bytesPerRow);
	}

	void MemoryGrid::Draw(const TArray<BigBestFitArena::Slot>& freeSlots)
	{
		String scaleStr      = Strings::ParseMemorySize(memoryScale);
		u32 scaleMultiplier  = u32(p::Log2(memoryScale));
		static const u32 min = 2, max = 8;
		UI::SliderScalar(
		    "Scale", ImGuiDataType_U32, (void*)&scaleMultiplier, &min, &max, scaleStr.c_str());
		memoryScale = p::Pow(2, scaleMultiplier);

		UI::BeginChild("##memoryblock", ImVec2(0.f, 450.f), false);
		UI::SetNextItemWidth(-FLT_MIN);
		DrawMemoryBlock("##MemoryGraph", *this, freeSlots, v2{0.f, 100.f});
		UI::EndChild();
	}

	BigBestFitArenaDebugger::BigBestFitArenaDebugger()
	{
		// TODO: Remove this. Testing the debugger
		if (auto* arena = dynamic_cast<BigBestFitArena*>(&GetCurrentArena()))
		{
			void* a = arena->Alloc(120);
			void* b = arena->Alloc(234);
			void* c = arena->Alloc(68);
			void* d = arena->Alloc(234);
			void* e = arena->Alloc(1522);
			void* f = arena->Alloc(344);
			void* g = arena->Alloc(33445);

			arena->Free(a, 120);
			arena->Free(c, 68);
			arena->Free(e, 1522);

			void* h = arena->Alloc(894345, 64);
		}
	}

	void BigBestFitArenaDebugger::Draw()
	{
		if (open)
		{
			auto* arena = dynamic_cast<BigBestFitArena*>(&GetCurrentArena());
			UI::Begin("Memory", &open);
			if (arena)
			{
				String size = Strings::ParseMemorySize(arena->GetBlock().size);
				String used = Strings::ParseMemorySize(arena->GetUsedSize());
				String free = Strings::ParseMemorySize(arena->GetFreeSize());

				{    // Progress bar
					const float usedPct = float(arena->GetUsedSize()) / arena->GetBlock().size;

					static const LinearColor lowMemoryColor  = LinearColor::FromRGB(56, 210, 41);
					static const LinearColor highMemoryColor = LinearColor::FromRGB(210, 56, 41);
					const LinearColor color =
					    LinearColor::LerpUsingHSV(lowMemoryColor, highMemoryColor, usedPct);
					UI::PushStyleColor(ImGuiCol_PlotHistogram, color);
					UI::PushStyleColor(ImGuiCol_FrameBg, color.Desaturate(0.4f));

					UI::PushItemWidth(-FLT_MIN);
					UI::ProgressBar(usedPct, ImVec2(0.f, 0.0f), "");
					UI::PopItemWidth();

					const String usedPctLabel =
					    Strings::Format("{:.0f}%% used ({})", usedPct * 100.f, used);
					const float pctFontSize = (UI::GetFontSize() * usedPctLabel.size()) / 2.f;
					UI::SameLine(UI::GetContentRegionAvail().x / 2 - pctFontSize / 2,
					    UI::GetStyle().ItemInnerSpacing.x / 2);
					UI::Text(usedPctLabel);

					UI::SetItemAllowOverlap();
					const float usedFontSize = (UI::GetFontSize() * size.size()) / 2.f;
					UI::SameLine(UI::GetContentRegionAvail().x - usedFontSize);
					UI::Text(size);

					UI::PopStyleColor(2);
				}

				UI::Separator();

				blockGrid.block = &arena->GetBlock();
				blockGrid.Draw(arena->GetFreeSlots());
			}
			UI::End();
		}
	}
}    // namespace rift::editor
