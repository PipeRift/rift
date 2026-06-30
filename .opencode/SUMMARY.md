## Goal
Interactive arena memory graph in PipeDebug.h — address-relative blocks, zoom/pan, ruler bar, hex/ASCII preview bands, allocation markers, thread-safe arena registry.

## Constraints & Preferences
- ImPlot removed; raw ImGui drawlist
- Address-relative block positioning, gaps = free space
- Ruler bar with adaptive ticks (major/half/quarter)
- Zoom wheel (1.3× toward cursor), drag to pan
- Order: Timeline, HEX, String, Arena Blocks (clearly delimited)
- 24px max width per byte (caps zoom, guarantees readable HEX/String)
- Details panel on right when arena selected
- All changes in Extern/Pipe submodule
- Clang + Ninja build

## Done
- `GetAllArenas()` in PipeMemory.h
- `Arena()`/`~Arena()` register/deregister directly in .cpp
- Registry: C array `sArenaRegistry[128]` + `sArenaCount` (breaks circular init)
- Registry statics BEFORE `currentArena`/`arenaStack`
- `GetAllArenas()` thread-safe via shared_mutex, returns TView
- ImPlot fully removed
- DrawMemory uses ImGui drawlist, address-relative
- DebugMemoryContext: zoom, pan, panning, panPos, panStart
- Ruler: `NiceRound()` (1/2/5×10^N), quarter-ticks, dark bg
- Every arena row renders (bg + label) even when GetBlocks empty
- Fixed: vertical double EndChild, ruler infinite loop, pan div-by-zero, arenaInfo scope
- `io.Fonts->Build()` moved to `UI::Init()` after backend init
- `AddArenaToRegistry` removed (base Arena()/~Arena() handle it)
- `GetBlocks()`: MonoLinearArena (IsAllocated guard), BestFitArena, BigBestFitArena, MultiLinearArena (via LinearBasePool::GetBlocks())
- **Unified graph child**: ruler, HEX, ASCII, rows all inside one interactable area
- **graphRect = full child canvas** → zoom/pan/select everywhere
- Vertical: hex/ASCII as left sub-columns (graphX0/graphW offset)
- Zoom in hex column anchors to view start
- **HEX/String drawn from zoomed visible range** (not full block) → bytes at correct screen position when zoomed in
- **Two delimited bands** (horizontal) / sub-columns (vertical): Timeline → HEX → ASCII → Arena Blocks, 2px gaps
- **Allocation markers on rows**: green ticks for live allocs, red ticks for freed-but-tracked
- **Pan with left or middle mouse** (left-drag pans, left-click selects; middle-drag also pans)
- **Zoom cap: 24px/byte max** (maxZoom = range * 24 / graphW)
- Toolbar `(?)` tooltip explains drag/wheel/click

## In Progress
- (none)

## Blocked / Known Issues
- `HeapArena` has no contiguous backing block; GetBlocks() empty → row visible, no rectangles, no alloc markers
- `Arena::Arena()` stores `entry.typeId = GetTypeId()` in base ctor; `Castable::typeId` caches `TypeId<Arena>` for all registered arenas. Labels may all show "Arena" until addressed.

## Key Decisions
- **ImPlot removed**: invisible rects. Pure ImGui drawlist.
- **Ruler spacing**: NiceRound 1/2/5×10^N, min 1.0 byte.
- **Zoom**: float, toward cursor. Cap by pixel density (24px/byte).
- **Pan**: left or middle drag, fraction of extra range.
- **Registry**: C array avoids static init circular dep.
- **Registration**: base Arena() registers, ~Arena() deregisters.
- **Thread safety**: shared_mutex on registry; stats mutex for allocation copy.
- **Two bands/columns**: HEX and ASCII each get their own 20px region with 2px gap; bandGap/colGap included in hexLineHeight/hexLineWidth.
- **24px cap**: computed each frame from range*24/graphW; guarantees HEX (2 chars) and ASCII (1 char) fit at max zoom.
- **Left-drag pans**: ImGui distinguishes click (select) from drag (pan) by threshold; both coexist.
- **Alloc markers**: thin (3px) top-edge ticks, clipped to arena range and graph area.

## Next Steps
1. Decide HeapArena visualization
2. Fix arena typeId resolution
3. Build test with Clang + Ninja
4. Verify HEX/String readability at 24px/byte

## Critical Context
- **Static init**: registry statics before arenaStack
- **MonoLinearArena::GetBlocks()**: IsAllocated guard
- **LinearBasePool**: GetBlocks() reports [lb, lb+blockSize)
- **Graph layout**: graphX0 = canvasPos.x + hexLineWidth, graphW = canvasSize.x - hexLineWidth
- **HEX/String mapping**: x = graphX0 + (addr - viewStart)/viewRange * graphW; threshold pixelsPerByte >= 4
- **Zoom cap**: maxZoom = range * 24.0 / graphW (floor 1.0)
- **Pan**: IsMouseDragging(0) || IsMouseDragging(2)
- **Alloc copy**: shared_lock on stats->mutex, copy TArrays
- **Language**: Chinese user
- **Build**: Clang 22.1.3 + VS Build Tools 18 + Ninja

## Relevant Files
- `Extern/Pipe/Include/PipeMemory.h` — Arena ctor/dtor decl, RegisteredArenaPtr, GetAllArenas
- `Extern/Pipe/Src/PipeMemory.cpp` — registry C array, Arena()/~Arena(), decl order, shared_mutex
- `Extern/Pipe/Include/PipeMemoryArenas.h` — GetBlocks() for all arenas + LinearBasePool
- `Extern/Pipe/Include/Misc/PipeDebug.h` — DrawMemory: two bands, alloc markers, left-pan, 24px zoom cap, hint
- `Libs/UI/Src/Style.cpp` — removed io.Fonts->Build()
- `Libs/UI/Src/Window.cpp` — io.Fonts->Build() after backend init
