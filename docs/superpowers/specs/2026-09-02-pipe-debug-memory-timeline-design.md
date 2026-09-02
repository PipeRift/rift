# PipeDebug Memory Timeline Graph — Design

Date: 2026-09-02
Status: Approved (pending spec review)

## Problem

PipeDebug's `DrawMemory` shows a static, address-space snapshot of all arenas as vertical columns. There is no way to see how memory **use** evolves over time. Add a horizontal time-series graph to the memory debugger showing, per arena, a capacity line and a used line.

## Scope

Add a fixed-height timeline graph to `DrawMemory` (in `Extern/Pipe/Include/Misc/PipeDebug.h`). Recruit per-frame `used`/`capacity` samples for arenas into a ring buffer and render them as lines over time. Changes stay within the Pipe submodule (`PipeDebug.h`, `MemoryStats.h`) and do **not** touch the other agent's in-progress refactor (MemoryStatsEvent tracking, tooltip cache, `FormatMemSize`).

## Feature Requirements

### Placement & layout

- A fixed-height panel rendered **right under the menu bar**, spanning the full width of the debugger window (above both the Details dock and the View graph).
- Fixed height (constant `timelineHeight`, ~100 px). Not resizable.
- The existing DockSpace/View/Details layout is unchanged and sits below it.

### Data recording (ring buffer)

- While the memory debugger window is open, at the start of each frame record a sample into each **live** arena snapshot: `time`, `used`, `capacity`.
- Recording targets **live** snapshots only, never captured frames — even when the debugger is currently viewing a capture.
- Each sample: `{ double time; sizet used; sizet capacity; }`.
- Time accumulates from `ImGui::GetIO().DeltaTime` each recorded frame.
- Ring buffer length (in seconds) is configurable in the settings submenu. Default **30 s**.
- Samples older than the configured window are pruned.
- Zero recording cost when the debugger window is closed.

### Data model

`DebugMemoryContext` gains:

- `float timelineHeight` — fixed height of the timeline (default ~100 px).
- A per-arena sampling structure holding the ring buffer. Since `snapshots` rebuild each frame, samples are keyed by stable arena identity (`const Arena*`). Persist a parallel array keyed by pointer so samples survive rebuilds:
  - `struct ArenaTimeline { const Arena* arena; TArray<Sample> samples; double lastTime; }`
  - `TArray<ArenaTimeline> timelines;`
- `i32 timelineBufferSeconds = 30` — configurable buffer length, exposed in settings.
- Multi-select set keyed by `const Arena*`:
  - `TArray<const Arena*> timelineSelectedArenas;`

### Arena selection (multi-select)

- The timeline shows **Heap arena always**, plus any arena currently selected.
- Selection reuses the existing memory-graph arena selection state. Selecting an arena in the memory graph toggles it in (or into) the timeline multi-select set.
- A legend in a corner of the timeline lists the selected arenas (including Heap) with their colors. Clicking a legend entry deselects it for the timeline.
- Toggling: selecting an arena that's already in the set removes it; selecting a new one adds it.

### Rendering

Two lines per selected arena:

- **Memory used:** arena's color (`details::GetArenaColor(typeId)`), solid.
- **Memory capacity:** only if `capacity > 0` for that arena. Same hue as the arena color but darker (`Shade(...)`).
- **Heap arena never draws a capacity line** (heap reports no capacity).

Axes:

- **X:** time, seconds. Left = oldest, right = newest. The view is locked with the newest sample pinned to the right edge.
- **Y:** bytes, `0` to the max capacity (or max used, whichever is greater) among the visible arenas over the visible window.
- Draw axis tick labels for both axes and a small corner legend (arena color dot + name).

Consistent with the rest of `DrawMemory`, rendering uses `ImGui::GetWindowDrawList()` (`AddLineSegment`/`AddPolyline` between consecutive samples).

## Non-Goals

- No wall-clock timestamps (frame-accumulated time only).
- No capacity line for heap.
- No interaction on the timeline (no pan/zoom/selection over time) in this iteration.
- No shared global always-on recording (only records while the debugger is open).

## Implementation Notes

All changes inside `Extern/Pipe`:
- `Include/Misc/PipeDebug.h`:
  - Extend `DebugMemoryContext` with the timeline fields above.
  - Add sample struct + ring buffer storage.
  - In the live snapshot rebuild loop, record `used`/`capacity`/`time` into the arena's ring, pruning old samples.
  - In `DrawMemory`, after the menu bar and before the DockSpace, render the timeline panel.
  - Add a `Buffer (s)` length control to the settings submenu.
  - Multi-select: when an arena column is clicked in the memory graph, toggle it in `timelineSelectedArenas`.
- No changes to `MemoryStats.h` required unless capacity needs a new source — capacity comes from existing snapshot `capacity` (sum of block sizes); heap has none.

## Testing

- Manual via the Rift editor's `ArenaDebugger` (`DrawMemory("Memory", &open)`).
  - Open debugger, confirm timeline renders across full window width with Heap used line growing as allocations happen.
  - Select an arena in the memory graph; confirm its used + capacity (darker) lines appear.
  - Deselect; confirm its lines disappear, Heap stays.
  - Change buffer length in settings; confirm window length changes.
  - Confirm no capacity line for heap.
- Run `ctest` to ensure existing Bandit tests still pass (`RiftTests` links Pipe).
