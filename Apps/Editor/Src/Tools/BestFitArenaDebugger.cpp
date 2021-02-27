// Copyright 2015-2021 Piperift - All rights reserved

#include "Tools/BestFitArenaDebugger.h"

#include "Misc/Imgui.h"

#include <Math/Math.h>
#include <Memory/Arenas/GlobalArena.h>
#include <Strings/String.h>
#include <imgui_internal.h>


namespace Rift
{
	static constexpr Color freeColor{210, 56, 41};    // Red
	static constexpr Color usedColor{56, 210, 41};    // Green

	using namespace Memory;

	void DrawMemoryRect(ImGuiWindow* window, const MemoryGrid& grid, ImRect box, v2_u32 min,
	    v2_u32 max, const Color& color)
	{
		const v2 minPos = v2{min} * grid.unitSize;
		const v2 maxPos = (v2{max} * grid.unitSize) + grid.unitSize;
		window->DrawList->AddRectFilled(box.Min + minPos, box.Min + maxPos, color.ToPackedABGR());
	}

	i32 DrawMemoryBlock(StringView label, MemoryGrid& grid,
	    const TArray<BestFitArena::Slot>& freeSlots, v2 graphSize = v2::Zero())
	{
		ImGuiContext& g     = *GImGui;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return -1;
		}

		const ImGuiStyle& style = g.Style;
		const ImGuiID id        = window->GetID(label.data());
		const ImVec2 label_size = ImGui::CalcTextSize(label.data(), NULL, true);

		if (graphSize.x == 0.0f)
		{
			graphSize.x = ImGui::CalcItemWidth();
		}
		if (graphSize.y == 0.0f)
		{
			graphSize.y = label_size.y;
		}

		grid.UpdateGridScale(graphSize.x);
		graphSize.y = Math::Max(graphSize.y, grid.GetHeight());

		const ImRect frameBox(window->DC.CursorPos, window->DC.CursorPos + graphSize);
		const ImRect totalBox(frameBox.Min,
		    frameBox.Max +
		        ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
		ImGui::ItemSize(totalBox, style.FramePadding.y);
		if (!ImGui::ItemAdd(totalBox, 0, &frameBox))
		{
			return -1;
		}
		const bool hovered = ImGui::ItemHoverable(frameBox, id);

		ImGui::RenderFrame(frameBox.Min, frameBox.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true,
		    style.FrameRounding);

		// Render
		{
			DrawMemoryRect(
			    window, grid, frameBox, {0, 0}, {grid.numColumns - 1, grid.numRows - 2}, usedColor);
			DrawMemoryRect(window, grid, frameBox, {0, grid.numRows - 1},
			    {grid.GetX(grid.block->GetSize()), grid.numRows - 1}, usedColor);

			DrawMemoryRect(window, grid, frameBox, {0, 0}, {0, 0}, freeColor);

			for (auto& slot : freeSlots)
			{
				const sizet startOffset = grid.GetOffset(slot.start);
				const sizet endOffset   = grid.GetOffset(slot.end);
				const u32 startX        = grid.GetX(startOffset);
				const u32 endX          = grid.GetX(endOffset);
				const u32 startY        = grid.GetY(startOffset);
				const u32 endY          = Math::Min(grid.GetY(endOffset), grid.numRows - 1);

				// Draw incomplete rows
				if (startY != endY)
				{
					DrawMemoryRect(window, grid, frameBox, {startX, startY},
					    {grid.numColumns - 1, startY}, freeColor);

					DrawMemoryRect(window, grid, frameBox, {0, endY}, {endX, endY}, freeColor);

					// Draw full rows
					if (endY - startY > 1)
					{
						DrawMemoryRect(window, grid, frameBox, {0, startY + 1},
						    {grid.numColumns - 1, endY - 1}, freeColor);
					}
				}
				else
				{
					DrawMemoryRect(window, grid, frameBox, {startX, endY}, {endX, endY}, freeColor);
				}
			}
		}

		if (label_size.x > 0.0f)
		{
			ImGui::RenderText(
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
		numRows     = u32(block->GetSize() / bytesPerRow);
	}

	void MemoryGrid::Draw(const TArray<BestFitArena::Slot>& freeSlots)
	{
		String scaleStr      = CString::ParseMemorySize(memoryScale);
		u32 scaleMultiplier  = u32(Math::Log(memoryScale, 2.f));
		static const u32 min = 2, max = 8;
		ImGui::SliderScalar(
		    "Scale", ImGuiDataType_U32, (void*) &scaleMultiplier, &min, &max, scaleStr.c_str());
		memoryScale = Math::Pow(2, scaleMultiplier);

		ImGui::BeginChild("##memoryblock", ImVec2(0.f, 450.f), false);
		ImGui::SetNextItemWidth(-FLT_MIN);
		DrawMemoryBlock("##MemoryGraph", *this, freeSlots, v2{0.f, 100.f});
		ImGui::EndChild();
	}

	BestFitArenaDebugger::BestFitArenaDebugger()
	{
		auto& arena = GetGlobalArena();

		void* a = arena.Allocate(120);
		void* b = arena.Allocate(234);
		void* c = arena.Allocate(68);
		void* d = arena.Allocate(234);
		void* e = arena.Allocate(1522);
		void* f = arena.Allocate(344);
		void* g = arena.Allocate(33445);

		arena.Free(a);
		arena.Free(c);
		arena.Free(e);

		void* h = arena.Allocate(894345, 64);
	}

	void BestFitArenaDebugger::Draw()
	{
		if (open)
		{
			auto& arena = GetGlobalArena();

			ImGui::Begin("Memory", &open);

			String size = CString::ParseMemorySize(arena.GetBlock().GetSize());
			String used = CString::ParseMemorySize(arena.GetUsedSize());
			String free = CString::ParseMemorySize(arena.GetFreeSize());

			{    // Progress bar
				const float usedPct = float(arena.GetUsedSize()) / arena.GetBlock().GetSize();

				static const LinearColor lowMemoryColor  = Color(56, 210, 41);
				static const LinearColor highMemoryColor = Color(210, 56, 41);
				const LinearColor color =
				    LinearColor::LerpUsingHSV(lowMemoryColor, highMemoryColor, usedPct);
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, color.Desaturate(0.4f));

				ImGui::PushItemWidth(-FLT_MIN);
				ImGui::ProgressBar(usedPct, ImVec2(0.f, 0.0f), "");

				const String usedPctLabel =
				    CString::Format("{:.0f}%% used ({})", usedPct * 100.f, used);
				const float pctFontSize = (ImGui::GetFontSize() * usedPctLabel.size()) / 2.f;
				ImGui::SameLine(ImGui::GetWindowContentRegionWidth() / 2 - pctFontSize / 2,
				    ImGui::GetStyle().ItemInnerSpacing.x / 2);
				ImGui::Text(usedPctLabel.c_str());

				ImGui::SetItemAllowOverlap();
				const float usedFontSize = (ImGui::GetFontSize() * size.size()) / 2.f;
				ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - usedFontSize);
				ImGui::Text(size.c_str());

				ImGui::PopStyleColor(2);
			}

			ImGui::Separator();

			blockGrid.block = &arena.GetBlock();
			blockGrid.Draw(arena.GetFreeSlots());

			ImGui::End();
		}
	}
}    // namespace Rift
