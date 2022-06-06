// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Core/Platform.h>
#include <Core/StringView.h>
#include <Math/Vector.h>
#include <Memory/Arenas/BigBestFitArena.h>
#include <Memory/Blocks/Block.h>


namespace rift
{
	using namespace p;

	struct MemoryGrid
	{
		static constexpr v2 unitSize{4.f, 4.f};    // Size of each grid point

		u32 memoryScale               = 8;    // Each gridpoint will equal N bytes
		u32 numColumns                = 0;
		u32 bytesPerRow               = 0;
		u32 numRows                   = 0;
		const p::Memory::Block* block = nullptr;


		MemoryGrid() = default;

		void UpdateGridScale(float availableWidth);

		void Draw(const TArray<p::Memory::BigBestFitArena::Slot>& freeSlots);

		float GetHeight()
		{
			return unitSize.y * numRows;
		}

		u32 GetY(sizet offset)
		{
			return u32(offset / bytesPerRow);
		}

		u32 GetX(sizet offset)
		{
			return (offset % bytesPerRow) / memoryScale;
		}
		sizet GetOffset(u8* ptr)
		{
			return ptr - static_cast<const u8*>(block->GetData());
		}
	};


	struct BigBestFitArenaDebugger
	{
		bool open = false;

		MemoryGrid blockGrid;


		BigBestFitArenaDebugger();
		void Draw();
	};
}    // namespace rift
