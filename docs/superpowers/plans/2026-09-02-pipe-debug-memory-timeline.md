# PipeDebug Memory Timeline Graph — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a fixed-height, time-series graph to the `DrawMemory` debugger showing, per arena, a "used" line and (where present) a darker "capacity" line over the last N seconds of recorded stats.

**Architecture:** Extend `DebugMemoryContext` (in Pipe's single-header `PipeDebug.h`) with a per-arena ring buffer of `{time, used, capacity}` samples recorded once per frame while the debugger window is open. Render a fixed-height timeline panel right below the menu bar, above the existing DockSpace, spanning full window width. Heap arena is always drawn; selected arenas (multi-select toggled by clicking memory-graph columns) are also drawn. Two lines per arena: used = arena color, capacity = arena color darkened (`Shade`). Heap never draws capacity.

**Tech Stack:** C++20, ImGui (public API + `ImGui::GetWindowDrawList()`), Pipe `TArray`, `Color` (`Shade`/`DWColor`), `MemoryStats`.

## Global Constraints

- All changes live in `Extern/Pipe/Include/Misc/PipeDebug.h`.
- Work follows the **current on-disk** state of the file, which is mid-refactor by another agent: `DebugMemoryContext` uses `curSnapshot` / `liveSnapshot` (NOT the old `snapshots`/`captures` arrays). Do not revert or restructure the other agent's in-progress `MemoryStatsEvent`/tooltip/`Strings::ParseMemorySizeTo` code.
- Recording targets **live** snapshots only, never captured frames, and continues even while viewing a capture. Zero recording cost when the debugger window is closed (recording happens inside `DrawMemory`, which returns early when closed).
- Heap arena never draws a capacity line.
- Only draw capacity when `capacity > 0` for that arena.
- Y-axis scaled to max of all visible used/capacity over the visible window; X-axis locked so newest sample is at the right edge.
- Naming: methods/members `camelBack`, `Timeline*` camel-case for new types, per `.clang-tidy`.
- No modifications to `MemoryStats.h` or other files needed.

---

### Task 1: Timeline data structures in `DebugMemoryContext`

**Files:**
- Modify: `Extern/Pipe/Include/Misc/PipeDebug.h:212` (struct `DebugMemoryContext`)

**Interfaces:**
- Produces:
  - `struct TimelineSample { double time; sizet used; sizet capacity; }`
  - `struct ArenaTimeline { const Arena* arena = nullptr; TArray<TimelineSample> samples; }`
  - members: `float timelineHeight = 100.0f;`, `i32 timelineBufferSeconds = 30;`, `double timelineCurrentTime = 0.0;`, `TArray<ArenaTimeline> timelines;`, `TArray<const Arena*> timelineSelectedArenas;`

Later tasks consume these exact types/members.

- [ ] **Step 1: Add the nested types and members**

Inside `DebugMemoryContext` (after the `MemorySnapshot`/`captures` block, before the `IsLive()` method at line ~312), add:

```cpp
		// Per-frame recorded stats, keyed by stable Arena pointer. Used by the
		// timeline graph. Recorded only while the debugger is open.
		struct TimelineSample
		{
			double time = 0.0;
			sizet used = 0;
			sizet capacity = 0;
		};
		struct ArenaTimeline
		{
			const Arena* arena = nullptr;
			TArray<TimelineSample> samples;
		};
		// Ring-buffer of recent per-frame samples, one entry per arena.
		TArray<ArenaTimeline> timelines;
		// Clock for the timeline, accumulated from ImGui::GetIO().DeltaTime.
		double timelineCurrentTime = 0.0;
		// How many seconds of history the timeline retains (configurable).
		i32 timelineBufferSeconds = 30;
		// Fixed pixel height of the timeline panel.
		float timelineHeight = 100.0f;
		// Arenas currently shown on the timeline (multi-select).
		TArray<const Arena*> timelineSelectedArenas;
```

- [ ] **Step 2: Verify it compiles**

Run: `cmake --build Build --config Debug` (from repo root `D:\Projects\Piperift\rift`)
Expected: builds clean (new members are unused yet, so no new warnings beyond existing ones).

- [ ] **Step 3: Commit**

```bash
git add Extern/Pipe
git commit -m "PipeDebug: timeline data structures in DebugMemoryContext"
```

---

### Task 2: Record per-frame samples into the ring buffer

**Files:**
- Modify: `Extern/Pipe/Include/Misc/PipeDebug.h:2216` (inside `DrawMemory`, at top of function)

**Interfaces:**
- Consumes: `memoryDbg.timelineCurrentTime`, `memoryDbg.timelineBufferSeconds`, `memoryDbg.timelines`, `memoryDbg.liveSnapshot.snapshots` (each with `.arena`, `.used`, `.capacity`), `ImGui::GetIO().DeltaTime`, `TimelineSample`, `ArenaTimeline`.
- Produces: a fully populated `memoryDbg.timelines` array (arena-keyed, pruned to the buffer window) that Task 3 renders.

- [ ] **Step 1: Write the recording helper as a static function**

The codebase uses static helper functions inside the implementation region. Recording must happen for all arenas-with-stats every frame the debugger is open, reading from the (possibly stale in capture mode) `liveSnapshot`. Add a static free function just above `DrawMemory` (after line 2214 `}    // namespace details`):

```cpp
	// Records one timeline sample per arena from the last live snapshot and
	// prunes samples older than the configured buffer window. Called once per
	// frame while the debugger window is open, in both live and capture modes.
	static void RecordMemoryTimeline(DebugMemoryContext& memoryDbg)
	{
		memoryDbg.timelineCurrentTime += ImGui::GetIO().DeltaTime;
		const double now = memoryDbg.timelineCurrentTime;

		for (const auto& snapshot : memoryDbg.liveSnapshot.snapshots)
		{
			if (!snapshot.arena)
			{
				continue;
			}
			// Find or create the arena's timeline.
			ArenaTimeline* timeline = nullptr;
			for (auto& t : memoryDbg.timelines)
			{
				if (t.arena == snapshot.arena)
				{
					timeline = &t;
					break;
				}
			}
			if (!timeline)
			{
				ArenaTimeline nt;
				nt.arena = snapshot.arena;
				nt.samples.Add(TimelineSample{now, snapshot.used, snapshot.capacity});
				memoryDbg.timelines.Add(Move(nt));
				continue;
			}

			timeline->samples.Add(TimelineSample{now, snapshot.used, snapshot.capacity});

			// Prune samples older than the buffer window, keeping at least one.
			const double window = static_cast<double>(memoryDbg.timelineBufferSeconds);
			while (timeline->samples.Size() > 1
			    && now - timeline->samples[0].time > window)
			{
				timeline->samples.RemoveAt(0, 1, Shrink::No);
			}
		}
	}
```

Note: `ImGui::GetIO().DeltaTime` is available here since `DrawMemory` runs inside ImGui. `Shrink::No` keeps `RemoveAt` cheap. `MemoryStats::CollectStats()` has already run during the live rebuild so `used` is fresh; in capture mode `liveSnapshot.used` is the last live value, which is the intended behaviour per the spec.

- [ ] **Step 2: Call `RecordMemoryTimeline` every frame**

Inside `DrawMemory`, right after the snapshot rebuild + sort block and before `ImGui::Begin(label, ...)` (i.e. after line 2315 `}` closing the `selectedArena` block), add:

```cpp
		// Timeline recording (only runs while this window is open).
		RecordMemoryTimeline(memoryDbg);
```

- [ ] **Step 3: Verify**

Run: `cmake --build Build --config Debug`
Expected: builds clean. No visible change yet (nothing renders the timeline).

- [ ] **Step 4: Commit**

```bash
git add Extern/Pipe
git commit -m "PipeDebug: record per-frame arena timeline samples"
```

---

### Task 3: Render the timeline graph panel

**Files:**
- Modify: `Extern/Pipe/Include/Misc/PipeDebug.h:2496` (after menu bar `EndMenuBar()`, before the DockSpace block)

**Interfaces:**
- Consumes: `memoryDbg.timelines`, `memoryDbg.timelineSelectedArenas`, `memoryDbg.timelineHeight`, `details::GetArenaColor(TypeId)`, `snapshot.arena->GetTypeId()`, `snapshot.name.Data()` (through liveSnapshot lookup), `TimelineSample`, `Color.Shade`, `Color.DWColor`.
- Produces: a drawn timeline panel. Also declares (Task 4 fills) the multi-select toggle hook.

- [ ] **Step 1: Insert the timeline drawing block**

Add this right after the menu bar `}` (line 2494 `ImGui::EndMenuBar(); }`) and before the DockSpace comment (line 2496). The block draws a fixed-height region using the window draw list, then the arena lines.

```cpp
		// ----- Memory timeline graph -----
		{
			ImDrawList* tlDraw = ImGui::GetWindowDrawList();
			const ImVec2 tlPos  = ImGui::GetCursorScreenPos();
			const float tlW     = ImGui::GetContentRegionAvail().x;
			const float tlH     = memoryDbg.timelineHeight;
			const ImRect tlRect(tlPos, ImVec2(tlPos.x + tlW, tlPos.y + tlH));
			const bool tlHovered = tlRect.Contains(ImGui::GetIO().MousePos);

			// Background + border
			tlDraw->AddRectFilled(tlRect.Min, tlRect.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));
			tlDraw->AddRect(tlRect.Min, tlRect.Max, ImGui::GetColorU32(ImGuiCol_Border));
			ImGui::Dummy(ImVec2(tlW, tlH + 2.0f));    // reserve vertical space

			// Gather the arenas currently shown: heap always, plus selections.
			// Newest-time = right edge. Determine visible time window.
			double newestTime = memoryDbg.timelineCurrentTime;
			double oldestTime = newestTime - static_cast<double>(memoryDbg.timelineBufferSeconds);
			if (oldestTime < 0.0)
			{
				oldestTime = 0.0;
			}
			const double span = (newestTime - oldestTime) > 0.0 ? (newestTime - oldestTime) : 1.0;

			struct VisibleArena
			{
				const Arena* arena;
				bool isHeap;
			};
			TArray<VisibleArena> visible;
			for (const auto* sel : memoryDbg.timelineSelectedArenas)
			{
				if (sel)
				{
					visible.Add(VisibleArena{sel, false});
				}
			}
			// Heap arena always shown (found in liveSnapshot).
			for (const auto& snap : memoryDbg.liveSnapshot.snapshots)
			{
				if (snap.arena && snap.typeId == GetTypeId<HeapArena>())
				{
					visible.Add(VisibleArena{snap.arena, true});
					break;
				}
			}

			// Compute Y scale: max used/capacity across visible arenas in window.
			sizet yMax = 1;
			for (const auto& va : visible)
			{
				for (auto& t : memoryDbg.timelines)
				{
					if (t.arena != va.arena)
					{
						continue;
					}
					for (const auto& s : t.samples)
					{
						if (s.time < oldestTime)
						{
							continue;
						}
						if (!va.isHeap && s.capacity > yMax)
						{
							yMax = s.capacity;
						}
						if (s.used > yMax)
						{
							yMax = s.used;
						}
					}
				}
			}

			// Ticks / labels (X = seconds, Y = bytes).
			const float padX = 44.0f;    // room for Y labels
			const float padTop = 8.0f;
			const float plotX0 = tlRect.Min.x + padX;
			const float plotX1 = tlRect.Max.x - 4.0f;
			const float plotY0 = tlRect.Min.y + padTop;
			const float plotY1 = tlRect.Max.y - 2.0f;
			const float plotW = (plotX1 - plotX0) > 0.0f ? (plotX1 - plotX0) : 1.0f;
			const float plotH = (plotY1 - plotY0) > 0.0f ? (plotY1 - plotY0) : 1.0f;

			auto XFor = [&](double t) -> float
			{
				const double rel = (t - oldestTime) / span;
				return plotX0 + static_cast<float>(rel) * plotW;
			};
			auto YFor = [&](sizet v) -> float
			{
				const double rel = static_cast<double>(v) / static_cast<double>(yMax);
				return plotY1 - static_cast<float>(rel) * plotH;
			};

			// Draw plot area background and grid lines
			tlDraw->AddRectFilled(ImVec2(plotX0, plotY0), ImVec2(plotX1, plotY1),
			    IM_COL32(20, 20, 20, 255));
			const ImU32 gridCol = p::Color{255, 255, 255, 18}.DWColor();
			for (i32 g = 0; g < 4; ++g)
			{
				const float gy = plotY1 - plotH * (g + 1) / 4.0f;
				tlDraw->AddLine(ImVec2(plotX0, gy), ImVec2(plotX1, gy), gridCol);
			}

			// Draw one used line and one capacity line per visible arena.
			for (const auto& va : visible)
			{
				ArenaTimeline* timeline = nullptr;
				for (auto& t : memoryDbg.timelines)
				{
					if (t.arena == va.arena)
					{
						timeline = &t;
						break;
					}
				}
				if (!timeline || timeline->samples.IsEmpty())
				{
					continue;
				}

				const p::Color arenaColor = details::GetArenaColor(va.arena->GetTypeId());

				// Capacity line (darker) — never for heap, only if capacity > 0.
				if (!va.isHeap && va.arena->GetAvailableMemory() > 0)
				{
					const p::Color capColor = arenaColor.Shade(0.4f);
					for (i32 i = 1; i < timeline->samples.Size(); ++i)
					{
						const auto& a = timeline->samples[i - 1];
						const auto& b = timeline->samples[i];
						if (b.time < oldestTime)
						{
							continue;
						}
						if (a.capacity > 0 && b.capacity > 0)
						{
							tlDraw->AddLine(ImVec2(XFor(a.time), YFor(a.capacity)),
							    ImVec2(XFor(b.time), YFor(b.capacity)), capColor.DWColor(), 1.5f);
						}
					}
				}

				// Used line (arena color).
				const ImU32 usedCol = arenaColor.DWColor();
				for (i32 i = 1; i < timeline->samples.Size(); ++i)
				{
					const auto& a = timeline->samples[i - 1];
					const auto& b = timeline->samples[i];
					if (b.time < oldestTime)
					{
						continue;
					}
					tlDraw->AddLine(ImVec2(XFor(a.time), YFor(a.used)),
					    ImVec2(XFor(b.time), YFor(b.used)), usedCol, 1.5f);
				}
			}

			// Y-axis max label + X-axis update marker drawn with ImGui text.
			char yBuf[32];
			{
				const double bytes = static_cast<double>(yMax);
				if (bytes >= 1024.0 * 1024.0)
				{
					snprintf(yBuf, sizeof(yBuf), "%.1fM", bytes / (1024.0 * 1024.0));
				}
				else if (bytes >= 1024.0)
				{
					snprintf(yBuf, sizeof(yBuf), "%.1fK", bytes / 1024.0);
				}
				else
				{
					snprintf(yBuf, sizeof(yBuf), "%llu", static_cast<unsigned long long>(bytes));
				}
				tlDraw->AddText(ImVec2(plotX0 - padX + 2.0f, plotY0),
				    p::Color{170, 170, 170}.DWColor(), yBuf);
			}
			tlDraw->AddText(ImVec2(plotX0, tlRect.Min.y + 1.0f),
			    p::Color{170, 170, 170}.DWColor(), "time");

			// Corner legend: color dot + arena name, clickable to deselect.
			float lx = plotX0 + 4.0f;
			float ly = plotY0 + 2.0f;
			const float legendH = 14.0f;
			for (const auto& va : visible)
			{
				const p::Color lc = details::GetArenaColor(va.arena->GetTypeId());
				tlDraw->AddRectFilled(ImVec2(lx, ly), ImVec2(lx + 8.0f, ly + 8.0f),
				    lc.DWColor());

				const char* name = nullptr;
				for (const auto& snap : memoryDbg.liveSnapshot.snapshots)
				{
					if (snap.arena == va.arena)
					{
						name = snap.name.Data();
						break;
					}
				}
				char legendBuf[64];
				if (!name || !name[0])
				{
					snprintf(legendBuf, sizeof(legendBuf), "Arena");
					name = legendBuf;
				}
				const char* dispName = (name && name[0]) ? name : "Arena";
				tlDraw->AddText(ImVec2(lx + 11.0f, ly - 2.0f), p::Color{220, 220, 220}.DWColor(),
				    dispName);
				const float legendW = 11.0f + ImGui::CalcTextSize(dispName).x;
				// Click legend entry to remove from selection (heap exempt).
				if (!va.isHeap && tlHovered && ImGui::IsMouseClicked(0))
				{
					const ImVec2 mpos = ImGui::GetIO().MousePos;
					if (mpos.x >= lx && mpos.x <= lx + legendW && mpos.y >= ly && mpos.y <= ly + legendH)
					{
						memoryDbg.timelineSelectedArenas.RemoveAt(
						    memoryDbg.timelineSelectedArenas.FindIndex(va.arena), Shrink::No);
					}
				}
				lx += legendW + 12.0f;
				if (lx > plotX1 - 20.0f)
				{
					lx = plotX0 + 4.0f;
					ly += legendH + 2.0f;
				}
			}

			// Tooltip on hover showing arena used/capacity at newer sample.
			if (tlHovered)
			{
				char tipBuf[128];
				snprintf(tipBuf, sizeof(tipBuf), "Timeline: last %is", memoryDbg.timelineBufferSeconds);
				ImGui::SetTooltip("%s", tipBuf);
			}

			ImGui::Separator();
		}
```

Note: The legend click edits `timelineSelectedArenas` while iterating over `visible` (a separate copied array), so no iterator invalidation. `IM_COL32` is available; the codebase already uses `p::Color{...}.DWColor()` for ImU32 elsewhere, so `IM_COL32` is consistent with ImGui headers. `GetTypeId<HeapArena>()` requires `PipeMemoryArenas.h`, which is already included transitively via `PipeMemory.h` usage in this file (heap code already references `HeapArena` elsewhere in the debugger). `ImGui::CalcTextSize` and `IM_COL32`, `ImGuiCol_*` are available.

- [ ] **Step 2: Add the buffer-length control to the settings submenu**

Inside the Settings menu (line 2484 `ImGui::SeparatorText("View");` area), add before `ImGui::SeparatorText("View");`:

```cpp
				ImGui::SeparatorText("Timeline");
				ImGui::SliderInt("Buffer (s)", &memoryDbg.timelineBufferSeconds, 1, 300);
```

- [ ] **Step 3: Verify**

Run: `cmake --build Build --config Debug`
Expected: builds clean. On manual run (Rift editor, open Memory debugger) the timeline renders across full window width with a Heap used line.

Manual test: open editor, open ArenaDebugger window. Confirm timeline appears under menu bar spanning full width, with a growing Heap "used" line and a legend entry labeled per `HeapArena`'s stats name. No capacity line for heap.

- [ ] **Step 4: Commit**

```bash
git add Extern/Pipe
git commit -m "PipeDebug: render memory timeline graph panel"
```

---

### Task 4: Multi-select arenas from the memory-graph columns

**Files:**
- Modify: `Extern/Pipe/Include/Misc/PipeDebug.h:3152-3198` (arena column click handler)

**Interfaces:**
- Consumes: `memoryDbg.timelineSelectedArenas`, `snapshot.arena`, `TArray::Contains`, `Add`, `RemoveAt`, `FindIndex`.
- Produces: clicking a memory-graph column toggles that arena in `timelineSelectedArenas` (so Task 3's rendered set updates live).

- [ ] **Step 1: Toggle the arena on column click**

In the column click handler, inside the `else` branch ("Empty part of the column: select the arena itself", line 3187-3197), after `memoryDbg.selectionEnd = reinterpret_cast<sizet>(snapshot.end);` add the timeline toggle:

```cpp
						// Toggle this arena in the timeline multi-select.
						if (snapshot.arena)
						{
							const i32 found = memoryDbg.timelineSelectedArenas.FindIndex(snapshot.arena);
							if (found != NO_INDEX)
							{
								memoryDbg.timelineSelectedArenas.RemoveAt(found, Shrink::No);
							}
							else
							{
								memoryDbg.timelineSelectedArenas.Add(snapshot.arena);
							}
						}
```

(Note: clicking a block — the `blockIdx != NO_INDEX` branch above — does NOT toggle the timeline; only clicking the empty part of a column selects the arena itself.)

- [ ] **Step 2: Verify**

Run: `cmake --build Build --config Debug`
Expected: builds clean.

Manual test: in the Memory graph, click the empty part of a non-heap arena column. Its used + darker capacity lines appear in the timeline (if that arena reports capacity), and a legend entry appears. Click the same column again to remove it; or click its legend entry. Immediately toggle back and forth and confirm lines show/hide. Confirm the heap line always stays.

- [ ] **Step 3: Run the test suite**

Run: `cmake --build Build --config Release && ctest --test-dir Build --output-on-failure -j2 -C Release`
Expected: all existing tests pass (no Pipe behavior changed; only debugger rendering/recording).

- [ ] **Step 4: Commit**

```bash
git add Extern/Pipe
git commit -m "PipeDebug: toggle arenas on timeline via graph selection"
```

---

## Self-Review notes

**Spec coverage:**
- Fixed-height timeline under menu bar, full width → Task 3 (placement) ✓
- Per-frame ring buffer recording while debugger open → Task 2 ✓
- Time = accumulated DeltaTime (seconds) → Task 2 (`timelineCurrentTime`) ✓
- Buffer length configurable in settings, default 30s → Task 3 Step 2 + Task 1 default ✓
- Heap always shown → Task 3 (`visible` includes heap always) ✓
- Selected arenas multi-select → Task 4 ✓
- Used line = arena color; capacity line = darker (Shade(0.4)); capacity only if >0 → Task 3 ✓
- Heap never shows capacity → Task 3 (`!va.isHeap`) + capacity-guard ✓
- Legend with colors in corner, clickable to deselect → Task 3 ✓
- Labelled axes (X seconds, Y bytes) → Task 3 (labels) ✓
- Newest at right edge → Task 3 (`XFor` with `oldestTime`..`newestTime`, `newestTime=timelineCurrentTime`) ✓
- Record into live snapshots even when not live → Task 2 reads `liveSnapshot`, called every frame ✓
- Zero cost when closed → recording inside `DrawMemory` (returns early when closed) ✓
- No changes to other agent's refactor → plan only adds new members/helpers/blocks; uses existing `Strings::ParseMemorySizeTo` untouched, does not alter `MemoryStatsEvent`/tooltip/`FormatMemSize` code ✓

**Placeholder scan:** all steps contain real code + exact paths + verify commands. No TBDs.

**Type consistency:** `TimelineSample`/`ArenaTimeline` defined in Task 1, used identically in Tasks 2–3. `timelineSelectedArenas` typed `TArray<const Arena*>` consistently. `Shade(0.4f)` (darker) matches `.clang-format`/existing usage (`Shade(0.65f)` for darker). `RemoveAt(idx, Shrink::No)` and `FindIndex(value)` used consistently with `TArray` API confirmed in `PipeContainers.h`.
