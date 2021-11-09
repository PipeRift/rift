// Copyright 2015-2021 Piperift - All rights reserved

// the structure of this file:
//
// [SECTION] bezier curve helpers
// [SECTION] draw list helper
// [SECTION] ui state logic
// [SECTION] render helpers
// [SECTION] API implementation

#include "UI/Nodes.h"

#include "UI/NodesInternal.h"
#include "UI/NodesMiniMap.h"

#include <Math/Bezier.h>
#include <Misc/Checks.h>

#include <cassert>
#include <climits>
#include <cmath>
#include <cstring>    // strlen, strncmp
#include <new>


namespace Rift::Nodes
{
	Context* gNodes = nullptr;

	EditorContext& GetEditorContext()
	{
		// No editor context was set! Did you forget to call Nodes::CreateContext()?
		assert(gNodes->EditorCtx != nullptr);
		return *gNodes->EditorCtx;
	}

	// [SECTION] bezier curve helpers

	CubicBezier MakeCubicBezier(
	    v2 start, v2 end, const PinType startType, const float lineSegmentsPerLength)
	{
		assert((startType == PinType::Input) || (startType == PinType::Output));
		if (startType == PinType::Input)
		{
			Swap(start, end);
		}

		const float linkLength = v2(end - start).Length();
		const v2 offset        = v2(0.25f * linkLength, 0.f);
		CubicBezier cubicBezier;
		cubicBezier.p0          = start;
		cubicBezier.p1          = start + offset;
		cubicBezier.p2          = end - offset;
		cubicBezier.p3          = end;
		cubicBezier.numSegments = ImMax(static_cast<i32>(linkLength * lineSegmentsPerLength), 1);
		return cubicBezier;
	}

	// Calculates the closest point along each bezier curve segment.
	v2 GetClosestPointOnCubicBezier(const i32 numSegments, const v2& p, const CubicBezier& cb)
	{
		Ensure(numSegments > 0);
		v2 pLast = cb.p0;
		v2 pClosest;
		float pClosestDist = FLT_MAX;
		float tStep        = 1.0f / (float)numSegments;
		for (i32 i = 1; i <= numSegments; ++i)
		{
			v2 pCurrent = Bezier::EvaluateCubic(cb.p0, cb.p1, cb.p2, cb.p3, tStep * i);
			v2 pLine    = Vectors::ClosestPointInLine(pLast, pCurrent, p);
			float dist  = (p - pLine).LengthSquared();
			if (dist < pClosestDist)
			{
				pClosest     = pLine;
				pClosestDist = dist;
			}
			pLast = pCurrent;
		}
		return pClosest;
	}

	float GetDistanceToCubicBezier(
	    const v2& pos, const CubicBezier& cubicBezier, const i32 numSegments)
	{
		const v2 pointOnCurve = GetClosestPointOnCubicBezier(numSegments, pos, cubicBezier);
		return (pointOnCurve - pos).Length();
	}

	Rect GetContainingRectForCubicBezier(const CubicBezier& cb)
	{
		const v2 min = v2(Math::Min(cb.p0.x, cb.p3.x), Math::Min(cb.p0.y, cb.p3.y));
		const v2 max = v2(Math::Max(cb.p0.x, cb.p3.x), Math::Max(cb.p0.y, cb.p3.y));

		const float hoverDistance = gNodes->Style.LinkHoverDistance;

		Rect rect(min, max);
		rect.Add(cb.p1);
		rect.Add(cb.p2);
		rect.Expand(v2{hoverDistance, hoverDistance});

		return rect;
	}

	float EvalImplicitLineEq(const v2& p1, const v2& p2, const v2& p)
	{
		return (p2.y - p1.y) * p.x + (p1.x - p2.x) * p.y + (p2.x * p1.y - p1.x * p2.y);
	}

	i32 Sign(float val)
	{
		return i32(val > 0.0f) - i32(val < 0.0f);
	}

	bool RectangleOverlapsLineSegment(const Rect& rect, const v2& p1, const v2& p2)
	{
		// Trivial case: rectangle contains an endpoint
		if (rect.Contains(p1) || rect.Contains(p2))
		{
			return true;
		}

		// Flip rectangle if necessary
		Rect flipRect = rect;

		if (flipRect.min.x > flipRect.max.x)
		{
			ImSwap(flipRect.min.x, flipRect.max.x);
		}

		if (flipRect.min.y > flipRect.max.y)
		{
			ImSwap(flipRect.min.y, flipRect.max.y);
		}

		// Trivial case: line segment lies to one particular side of rectangle
		if ((p1.x < flipRect.min.x && p2.x < flipRect.min.x)
		    || (p1.x > flipRect.max.x && p2.x > flipRect.max.x)
		    || (p1.y < flipRect.min.y && p2.y < flipRect.min.y)
		    || (p1.y > flipRect.max.y && p2.y > flipRect.max.y))
		{
			return false;
		}

		const i32 cornerSigns[4] = {Sign(EvalImplicitLineEq(p1, p2, flipRect.min)),
		    Sign(EvalImplicitLineEq(p1, p2, v2(flipRect.max.x, flipRect.min.y))),
		    Sign(EvalImplicitLineEq(p1, p2, v2(flipRect.min.x, flipRect.max.y))),
		    Sign(EvalImplicitLineEq(p1, p2, flipRect.max))};

		i32 sum    = 0;
		i32 sumAbs = 0;

		for (int cornerSign : cornerSigns)
		{
			sum += cornerSign;
			sumAbs += abs(cornerSign);
		}

		// At least one corner of rectangle lies on a different side of line segment
		return abs(sum) != sumAbs;
	}

	bool RectangleOverlapsBezier(const Rect& rectangle, const CubicBezier& cubicBezier)
	{
		v2 current = Bezier::EvaluateCubic(
		    cubicBezier.p0, cubicBezier.p1, cubicBezier.p2, cubicBezier.p3, 0.f);

		const float dt = 1.0f / cubicBezier.numSegments;
		for (i32 s = 0; s < cubicBezier.numSegments; ++s)
		{
			const v2 next = Bezier::EvaluateCubic(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
			    cubicBezier.p3, float((s + 1) * dt));
			if (RectangleOverlapsLineSegment(rectangle, current, next))
			{
				return true;
			}
			current = next;
		}
		return false;
	}

	bool RectangleOverlapsLink(
	    const Rect& rectangle, const v2& start, const v2& end, const PinType startType)
	{
		// First level: simple rejection test via rectangle overlap:

		Rect lrect = Rect(start, end);
		if (lrect.min.x > lrect.max.x)
		{
			ImSwap(lrect.min.x, lrect.max.x);
		}

		if (lrect.min.y > lrect.max.y)
		{
			ImSwap(lrect.min.y, lrect.max.y);
		}

		if (rectangle.Overlaps(lrect))
		{
			// First, check if either one or both endpoinds are trivially contained
			// in the rectangle

			if (rectangle.Contains(start) || rectangle.Contains(end))
			{
				return true;
			}

			// Second level of refinement: do a more expensive test against the
			// link

			const CubicBezier cubicBezier =
			    MakeCubicBezier(start, end, startType, gNodes->Style.linkLineSegmentsPerLength);
			return RectangleOverlapsBezier(rectangle, cubicBezier);
		}

		return false;
	}

	// [SECTION] coordinate space conversion helpers


	v2 EditorToScreenPosition(const v2& v)
	{
		return gNodes->CanvasOriginScreenSpace + v;
	}

	v2 ScreenToGridPosition(const EditorContext& editor, const v2& v)
	{
		return v - gNodes->CanvasOriginScreenSpace - editor.Panning;
	}

	v2 GridToScreenPosition(const EditorContext& editor, const v2& v)
	{
		return v + gNodes->CanvasOriginScreenSpace + editor.Panning;
	}

	v2 GridToEditorPosition(const EditorContext& editor, const v2& v)
	{
		return v + editor.Panning;
	}

	v2 EditorToGridPosition(const EditorContext& editor, const v2& v)
	{
		return v - editor.Panning;
	}

	v2 MiniMapToGridPosition(const EditorContext& editor, const v2& v)
	{
		return (v - editor.miniMap.contentScreenSpace.min) / editor.miniMap.scaling
		     + editor.gridContentBounds.min;
	};

	v2 ScreenToMiniMapPosition(const EditorContext& editor, const v2& v)
	{
		return (ScreenToGridPosition(editor, v) - editor.gridContentBounds.min)
		         * editor.miniMap.scaling
		     + editor.miniMap.contentScreenSpace.min;
	};

	v2 ScreenToGridPosition(const v2& v)
	{
		return ScreenToGridPosition(GetEditorContext(), v);
	}
	v2 GridToScreenPosition(const v2& v)
	{
		return GridToScreenPosition(GetEditorContext(), v);
	}
	v2 GridToEditorPosition(const v2& v)
	{
		return GridToEditorPosition(GetEditorContext(), v);
	}
	v2 EditorToGridPosition(const v2& v)
	{
		return EditorToGridPosition(GetEditorContext(), v);
	}
	v2 MiniMapToGridPosition(const v2& v)
	{
		return MiniMapToGridPosition(GetEditorContext(), v);
	}
	v2 ScreenToMiniMapPosition(const v2& v)
	{
		return ScreenToMiniMapPosition(GetEditorContext(), v);
	}


	// [SECTION] draw list helper

	void ImDrawListGrowChannels(ImDrawList* drawList, const i32 numChannels)
	{
		ImDrawListSplitter& splitter = drawList->_Splitter;

		if (splitter._Count == 1)
		{
			splitter.Split(drawList, numChannels + 1);
			return;
		}

		// NOTE: this logic has been lifted from ImDrawListSplitter::Split with slight
		// modifications to allow nested splits. The main modification is that we only create
		// new ImDrawChannel instances after splitter._Count, instead of over the whole
		// splitter._Channels array like the regular ImDrawListSplitter::Split method does.

		const i32 oldChannelCapacity = splitter._Channels.Size;
		// NOTE: _Channels is not resized down, and therefore _Count <= _Channels.size()!
		const i32 oldChannelCount       = splitter._Count;
		const i32 requestedChannelCount = oldChannelCount + numChannels;
		if (oldChannelCapacity < oldChannelCount + numChannels)
		{
			splitter._Channels.resize(requestedChannelCount);
		}

		splitter._Count = requestedChannelCount;

		for (i32 i = oldChannelCount; i < requestedChannelCount; ++i)
		{
			ImDrawChannel& channel = splitter._Channels[i];

			// If we're inside the old capacity region of the array, we need to reuse the
			// existing memory of the command and index buffers.
			if (i < oldChannelCapacity)
			{
				channel._CmdBuffer.resize(0);
				channel._IdxBuffer.resize(0);
			}
			// Else, we need to construct new draw channels.
			else
			{
				IM_PLACEMENT_NEW(&channel) ImDrawChannel();
			}

			{
				ImDrawCmd drawCmd;
				drawCmd.ClipRect  = drawList->_ClipRectStack.back();
				drawCmd.TextureId = drawList->_TextureIdStack.back();
				channel._CmdBuffer.push_back(drawCmd);
			}
		}
	}

	void ImDrawListSplitterSwapChannels(
	    ImDrawListSplitter& splitter, const i32 lhsIdx, const i32 rhsIdx)
	{
		if (lhsIdx == rhsIdx)
		{
			return;
		}

		assert(lhsIdx >= 0 && lhsIdx < splitter._Count);
		assert(rhsIdx >= 0 && rhsIdx < splitter._Count);

		ImDrawChannel& lhsChannel = splitter._Channels[lhsIdx];
		ImDrawChannel& rhsChannel = splitter._Channels[rhsIdx];
		lhsChannel._CmdBuffer.swap(rhsChannel._CmdBuffer);
		lhsChannel._IdxBuffer.swap(rhsChannel._IdxBuffer);

		const i32 currentChannel = splitter._Current;

		if (currentChannel == lhsIdx)
		{
			splitter._Current = rhsIdx;
		}
		else if (currentChannel == rhsIdx)
		{
			splitter._Current = lhsIdx;
		}
	}

	void DrawListSet(ImDrawList* windowDrawList)
	{
		gNodes->CanvasDrawList = windowDrawList;
		gNodes->NodeIdxToSubmissionIdx.Clear();
		gNodes->NodeIdxSubmissionOrder.clear();
	}

	// The draw list channels are structured as follows. First we have our base channel, the
	// canvas grid on which we render the grid lines in BeginNodeEditor(). The base channel is
	// the reason draw_list_submission_idx_to_background_channel_idx offsets the index by one.
	// Each BeginNode() call appends two new draw channels, for the node background and
	// foreground. The node foreground is the channel i32o which the node's ImGui content is
	// rendered. Finally, in EndNodeEditor() we append one last draw channel for rendering the
	// selection box and the incomplete link on top of everything else.
	//
	// +----------+----------+----------+----------+----------+----------+
	// |          |          |          |          |          |          |
	// |canvas    |node      |node      |...       |...       |click     |
	// |grid      |background|foreground|          |          |i32eraction
	// |          |          |          |          |          |          |
	// +----------+----------+----------+----------+----------+----------+
	//            |                     |
	//            |   submission idx    |
	//            |                     |
	//            -----------------------

	void DrawListAddNode(const i32 nodeIdx)
	{
		gNodes->NodeIdxToSubmissionIdx.SetInt(
		    static_cast<ImGuiID>(nodeIdx), gNodes->NodeIdxSubmissionOrder.Size);
		gNodes->NodeIdxSubmissionOrder.push_back(nodeIdx);
		ImDrawListGrowChannels(gNodes->CanvasDrawList, 2);
	}

	void DrawListAppendClickInteractionChannel()
	{
		// NOTE: don't use this function outside of EndNodeEditor. Using this before all nodes
		// have been added will screw up the node draw order.
		ImDrawListGrowChannels(gNodes->CanvasDrawList, 1);
	}

	i32 DrawListSubmissionIdxToBackgroundChannelIdx(const i32 submissionIdx)
	{
		// NOTE: the first channel is the canvas background, i.e. the grid
		return 1 + 2 * submissionIdx;
	}

	i32 DrawListSubmissionIdxToForegroundChannelIdx(const i32 submissionIdx)
	{
		return DrawListSubmissionIdxToBackgroundChannelIdx(submissionIdx) + 1;
	}

	void DrawListActivateClickInteractionChannel()
	{
		gNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    gNodes->CanvasDrawList, gNodes->CanvasDrawList->_Splitter._Count - 1);
	}

	void DrawListActivateCurrentNodeForeground()
	{
		const i32 foregroundChannelIdx =
		    DrawListSubmissionIdxToForegroundChannelIdx(gNodes->NodeIdxSubmissionOrder.Size - 1);
		gNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    gNodes->CanvasDrawList, foregroundChannelIdx);
	}

	void DrawListActivateNodeBackground(const i32 nodeIdx)
	{
		const i32 submissionIdx =
		    gNodes->NodeIdxToSubmissionIdx.GetInt(static_cast<ImGuiID>(nodeIdx), -1);
		// There is a discrepancy in the submitted node count and the rendered node count! Did
		// you call one of the following functions
		// * EditorContextMoveToNode
		// * SetNodeScreenSpacePos
		// * SetNodeGridSpacePos
		// * SetNodeDraggable
		// after the BeginNode/EndNode function calls?
		assert(submissionIdx != -1);
		const i32 backgroundChannelIdx = DrawListSubmissionIdxToBackgroundChannelIdx(submissionIdx);
		gNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    gNodes->CanvasDrawList, backgroundChannelIdx);
	}

	void DrawListSwapSubmissionIndices(const i32 lhsIdx, const i32 rhsIdx)
	{
		assert(lhsIdx != rhsIdx);

		const i32 lhsForegroundChannelIdx = DrawListSubmissionIdxToForegroundChannelIdx(lhsIdx);
		const i32 lhsBackgroundChannelIdx = DrawListSubmissionIdxToBackgroundChannelIdx(lhsIdx);
		const i32 rhsForegroundChannelIdx = DrawListSubmissionIdxToForegroundChannelIdx(rhsIdx);
		const i32 rhsBackgroundChannelIdx = DrawListSubmissionIdxToBackgroundChannelIdx(rhsIdx);

		ImDrawListSplitterSwapChannels(
		    gNodes->CanvasDrawList->_Splitter, lhsBackgroundChannelIdx, rhsBackgroundChannelIdx);
		ImDrawListSplitterSwapChannels(
		    gNodes->CanvasDrawList->_Splitter, lhsForegroundChannelIdx, rhsForegroundChannelIdx);
	}

	void DrawListSortChannelsByDepth(const ImVector<i32>& nodeIdxDepthOrder)
	{
		if (gNodes->NodeIdxToSubmissionIdx.Data.Size < 2)
		{
			return;
		}

		assert(nodeIdxDepthOrder.Size == gNodes->NodeIdxSubmissionOrder.Size);

		i32 startIdx = nodeIdxDepthOrder.Size - 1;

		while (nodeIdxDepthOrder[startIdx] == gNodes->NodeIdxSubmissionOrder[startIdx])
		{
			if (--startIdx == 0)
			{
				// early out if submission order and depth order are the same
				return;
			}
		}

		// TODO: this is an O(N^2) algorithm. It might be worthwhile revisiting this to see if
		// the time complexity can be reduced.

		for (i32 depthIdx = startIdx; depthIdx > 0; --depthIdx)
		{
			const i32 nodeIdx = nodeIdxDepthOrder[depthIdx];

			// Find the current index of the nodeIdx in the submission order array
			i32 submissionIdx = -1;
			for (i32 i = 0; i < gNodes->NodeIdxSubmissionOrder.Size; ++i)
			{
				if (gNodes->NodeIdxSubmissionOrder[i] == nodeIdx)
				{
					submissionIdx = i;
					break;
				}
			}
			assert(submissionIdx >= 0);

			if (submissionIdx == depthIdx)
			{
				continue;
			}

			for (i32 j = submissionIdx; j < depthIdx; ++j)
			{
				DrawListSwapSubmissionIndices(j, j + 1);
				ImSwap(gNodes->NodeIdxSubmissionOrder[j], gNodes->NodeIdxSubmissionOrder[j + 1]);
			}
		}
	}

	// [SECTION] ui state logic

	v2 GetScreenSpacePinCoordinates(
	    const Rect& nodeRect, const Rect& attributeRect, const PinType type)
	{
		assert(type == PinType::Input || type == PinType::Output);
		const float x = type == PinType::Input ? (nodeRect.min.x - gNodes->Style.PinOffset)
		                                       : (nodeRect.max.x + gNodes->Style.PinOffset);
		return {x, 0.5f * (attributeRect.min.y + attributeRect.max.y)};
	}

	v2 GetScreenSpacePinCoordinates(const EditorContext& editor, const PinData& pin)
	{
		const Rect& parentNodeRect = editor.nodes.Pool[pin.ParentNodeIdx].Rect;
		return GetScreenSpacePinCoordinates(parentNodeRect, pin.rect, pin.Type);
	}

	bool IsMouseInCanvas()
	{
		// This flag should be true either when hovering or clicking something in the canvas.
		const bool isWindowHoveredOrFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();

		return isWindowHoveredOrFocused
		    && gNodes->CanvasRectScreenSpace.Contains(ImGui::GetMousePos());
	}

	void BeginNodeSelection(EditorContext& editor, const i32 nodeIdx)
	{
		// Don't start selecting a node if we are e.g. already creating and dragging
		// a new link! New link creation can happen when the mouse is clicked over
		// a node, but within the hover radius of a pin.
		if (editor.clickInteraction.Type != ClickInteractionType_None)
		{
			return;
		}

		editor.clickInteraction.Type = ClickInteractionType_Node;
		// If the node is not already contained in the selection, then we want only
		// the i32eraction node to be selected, effective immediately.
		//
		// If the multiple selection modifier is active, we want to add this node
		// to the current list of selected nodes.
		//
		// Otherwise, we want to allow for the possibility of multiple nodes to be
		// moved at once.
		if (!editor.SelectedNodeIndices.contains(nodeIdx))
		{
			editor.SelectedLinkIndices.clear();
			if (!gNodes->multipleSelectModifier)
				editor.SelectedNodeIndices.clear();
			editor.SelectedNodeIndices.push_back(nodeIdx);

			// Ensure that individually selected nodes get rendered on top
			ImVector<i32>& depthStack = editor.NodeDepthOrder;
			const i32* const elem     = depthStack.find(nodeIdx);
			assert(elem != depthStack.end());
			depthStack.erase(elem);
			depthStack.push_back(nodeIdx);
		}
		// Deselect a previously-selected node
		else if (gNodes->multipleSelectModifier)
		{
			const i32* const nodePtr = editor.SelectedNodeIndices.find(nodeIdx);
			editor.SelectedNodeIndices.erase(nodePtr);

			// Don't allow dragging after deselecting
			editor.clickInteraction.Type = ClickInteractionType_None;
		}

		// To support snapping of multiple nodes, we need to store the offset of
		// each node in the selection to the origin of the dragged node.
		const v2 refOrigin = editor.nodes.Pool[nodeIdx].Origin;
		editor.PrimaryNodeOffset =
		    refOrigin + gNodes->CanvasOriginScreenSpace + editor.Panning - gNodes->mousePosition;

		editor.SelectedNodeOrigins.clear();
		for (i32 idx = 0; idx < editor.SelectedNodeIndices.Size; idx++)
		{
			const i32 node      = editor.SelectedNodeIndices[idx];
			const v2 nodeOrigin = editor.nodes.Pool[node].Origin - refOrigin;
			editor.SelectedNodeOrigins.push_back(nodeOrigin);
		}
	}

	void BeginLinkSelection(EditorContext& editor, const i32 linkIdx)
	{
		editor.clickInteraction.Type = ClickInteractionType_Link;
		// When a link is selected, clear all other selections, and insert the link
		// as the sole selection.
		editor.SelectedNodeIndices.clear();
		editor.SelectedLinkIndices.clear();
		editor.SelectedLinkIndices.push_back(linkIdx);
	}

	void BeginLinkDetach(EditorContext& editor, const i32 linkIdx, const i32 detachPinIdx)
	{
		const LinkData& link         = editor.Links.Pool[linkIdx];
		ClickInteractionState& state = editor.clickInteraction;
		state.Type                   = ClickInteractionType_LinkCreation;
		state.LinkCreation.inputPinIdx.Reset();
		state.LinkCreation.outputPinIdx =
		    detachPinIdx == link.outputPinIdx ? link.inputPinIdx : link.outputPinIdx;
		gNodes->DeletedLinkIdx = linkIdx;
	}

	void BeginLinkCreation(EditorContext& editor, const i32 hoveredPinIdx)
	{
		editor.clickInteraction.Type                      = ClickInteractionType_LinkCreation;
		editor.clickInteraction.LinkCreation.outputPinIdx = hoveredPinIdx;
		editor.clickInteraction.LinkCreation.inputPinIdx.Reset();
		editor.clickInteraction.LinkCreation.Type = LinkCreationType_Standard;
		gNodes->UIState |= UIState_LinkStarted;
	}

	void BeginLinkInteraction(
	    EditorContext& editor, const i32 linkIdx, const OptionalIndex pinIdx = OptionalIndex())
	{
		// Check if we are clicking the link with the modifier pressed.
		// This will in a link detach via clicking.

		const bool modifierPressed = gNodes->Io.linkDetachWithModifierClick.modifier == nullptr
		                               ? false
		                               : *gNodes->Io.linkDetachWithModifierClick.modifier;

		if (modifierPressed)
		{
			const LinkData& link    = editor.Links.Pool[linkIdx];
			const PinData& startPin = editor.pins.Pool[link.outputPinIdx];
			const PinData& endPin   = editor.pins.Pool[link.inputPinIdx];
			const v2& mousePos      = gNodes->mousePosition;
			const float distToStart = ImLengthSqr(startPin.Pos - mousePos);
			const float distToEnd   = ImLengthSqr(endPin.Pos - mousePos);
			const i32 closestPinIdx =
			    distToStart < distToEnd ? link.outputPinIdx : link.inputPinIdx;

			editor.clickInteraction.Type = ClickInteractionType_LinkCreation;
			BeginLinkDetach(editor, linkIdx, closestPinIdx);
			editor.clickInteraction.LinkCreation.Type = LinkCreationType_FromDetach;
		}
		else
		{
			if (pinIdx.IsValid())
			{
				const i32 hoveredPinFlags = editor.pins.Pool[pinIdx.Value()].Flags;

				// Check the 'click and drag to detach' case.
				if (hoveredPinFlags & PinFlags_EnableLinkDetachWithDragClick)
				{
					BeginLinkDetach(editor, linkIdx, pinIdx.Value());
					editor.clickInteraction.LinkCreation.Type = LinkCreationType_FromDetach;
				}
				else
				{
					BeginLinkCreation(editor, pinIdx.Value());
				}
			}
			else
			{
				BeginLinkSelection(editor, linkIdx);
			}
		}
	}

	static bool IsMiniMapHovered();

	void BeginCanvasInteraction(EditorContext& editor)
	{
		const bool anyUiElementHovered =
		    gNodes->HoveredNodeIdx.IsValid() || gNodes->HoveredLinkIdx.IsValid()
		    || gNodes->HoveredPinIdx.IsValid() || ImGui::IsAnyItemHovered();

		const bool mouseNotInCanvas = !IsMouseInCanvas();

		if (editor.clickInteraction.Type != ClickInteractionType_None || anyUiElementHovered
		    || mouseNotInCanvas)
		{
			return;
		}

		const bool startedPanning = gNodes->AltMouseClicked;

		if (startedPanning)
		{
			editor.clickInteraction.Type = ClickInteractionType_Panning;
		}
		else if (gNodes->LeftMouseClicked)
		{
			editor.clickInteraction.Type = ClickInteractionType_BoxSelection;
			editor.clickInteraction.BoxSelector.Rect.min =
			    ScreenToGridPosition(editor, gNodes->mousePosition);
		}
	}

	void BoxSelectorUpdateSelection(EditorContext& editor, Rect boxRect)
	{
		// Invert box selector coordinates as needed

		if (boxRect.min.x > boxRect.max.x)
		{
			ImSwap(boxRect.min.x, boxRect.max.x);
		}

		if (boxRect.min.y > boxRect.max.y)
		{
			ImSwap(boxRect.min.y, boxRect.max.y);
		}

		// Update node selection

		editor.SelectedNodeIndices.clear();

		// Test for overlap against node rectangles

		for (i32 nodeIdx = 0; nodeIdx < editor.nodes.Pool.Size(); ++nodeIdx)
		{
			if (editor.nodes.InUse.IsSet(nodeIdx))
			{
				NodeData& node = editor.nodes.Pool[nodeIdx];
				if (boxRect.Overlaps(node.Rect))
				{
					editor.SelectedNodeIndices.push_back(nodeIdx);
				}
			}
		}

		// Update link selection

		editor.SelectedLinkIndices.clear();

		// Test for overlap against links

		for (i32 linkIdx = 0; linkIdx < editor.Links.Pool.Size(); ++linkIdx)
		{
			if (editor.Links.InUse.IsSet(linkIdx))
			{
				const LinkData& link = editor.Links.Pool[linkIdx];

				const PinData& pinStart   = editor.pins.Pool[link.outputPinIdx];
				const PinData& pinEnd     = editor.pins.Pool[link.inputPinIdx];
				const Rect& nodeStartRect = editor.nodes.Pool[pinStart.ParentNodeIdx].Rect;
				const Rect& nodeEndRect   = editor.nodes.Pool[pinEnd.ParentNodeIdx].Rect;

				const v2 start =
				    GetScreenSpacePinCoordinates(nodeStartRect, pinStart.rect, pinStart.Type);
				const v2 end = GetScreenSpacePinCoordinates(nodeEndRect, pinEnd.rect, pinEnd.Type);

				// Test
				if (RectangleOverlapsLink(boxRect, start, end, pinStart.Type))
				{
					editor.SelectedLinkIndices.push_back(linkIdx);
				}
			}
		}
	}

	v2 SnapOrigi32oGrid(v2 origin)
	{
		if ((gNodes->Style.Flags & StyleFlags_GridSnapping)
		    || ((gNodes->Style.Flags & StyleFlags_GridSnappingOnRelease)
		        && gNodes->leftMouseReleased))
		{
			const float spacing  = gNodes->Style.GridSpacing;
			const float spacing2 = spacing / 2.0f;
			float modx           = fmodf(fabsf(origin.x) + spacing2, spacing) - spacing2;
			float mody           = fmodf(fabsf(origin.y) + spacing2, spacing) - spacing2;
			origin.x += (origin.x < 0.f) ? modx : -modx;
			origin.y += (origin.y < 0.f) ? mody : -mody;
		}

		return origin;
	}

	void TranslateSelectedNodes(EditorContext& editor)
	{
		if (gNodes->leftMouseDragging || gNodes->leftMouseReleased)
		{
			const v2 origin =
			    SnapOrigi32oGrid(gNodes->mousePosition - gNodes->CanvasOriginScreenSpace
			                     - editor.Panning + editor.PrimaryNodeOffset);
			for (i32 i = 0; i < editor.SelectedNodeIndices.size(); ++i)
			{
				const v2 nodeRel  = editor.SelectedNodeOrigins[i];
				const i32 nodeIdx = editor.SelectedNodeIndices[i];
				NodeData& node    = editor.nodes.Pool[nodeIdx];
				if (node.Draggable)
				{
					// node.Origin += io.MouseDelta - editor.AutoPanningDelta;
					node.Origin = origin + nodeRel;
				}
			}
		}
	}

	struct LinkPredicate
	{
		bool operator()(const LinkData& lhs, const LinkData& rhs) const
		{
			// Do a unique compare by sorting the pins' addresses.
			// This catches duplicate links, whether they are in the
			// same direction or not.
			// Sorting by pin index should have the uniqueness guarantees as sorting
			// by id -- each unique id will get one slot in the link pool array.

			i32 lhsStart = lhs.outputPinIdx;
			i32 lhsEnd   = lhs.inputPinIdx;
			i32 rhsStart = rhs.outputPinIdx;
			i32 rhsEnd   = rhs.inputPinIdx;

			if (lhsStart > lhsEnd)
			{
				ImSwap(lhsStart, lhsEnd);
			}

			if (rhsStart > rhsEnd)
			{
				ImSwap(rhsStart, rhsEnd);
			}

			return lhsStart == rhsStart && lhsEnd == rhsEnd;
		}
	};

	OptionalIndex FindDuplicateLink(
	    const EditorContext& editor, const i32 startPinIdx, const i32 endPinIdx)
	{
		LinkData testLink(0);
		testLink.outputPinIdx = startPinIdx;
		testLink.inputPinIdx  = endPinIdx;
		for (i32 linkIdx = 0; linkIdx < editor.Links.Pool.Size(); ++linkIdx)
		{
			const LinkData& link = editor.Links.Pool[linkIdx];
			if (LinkPredicate()(testLink, link) && editor.Links.InUse.IsSet(linkIdx))
			{
				return OptionalIndex(linkIdx);
			}
		}

		return {};
	}

	bool ShouldLinkSnapToPin(const EditorContext& editor, const PinData& startPin,
	    const i32 hoveredPinIdx, const OptionalIndex duplicateLink)
	{
		const PinData& endPin = editor.pins.Pool[hoveredPinIdx];

		// The end pin must be in a different node
		if (startPin.ParentNodeIdx == endPin.ParentNodeIdx)
		{
			return false;
		}

		// The end pin must be of a different type
		if (startPin.Type == endPin.Type)
		{
			return false;
		}

		// The link to be created must not be a duplicate, unless it is the link which was
		// created on snap. In that case we want to snap, since we want it to appear visually as
		// if the created link remains snapped to the pin.
		if (duplicateLink.IsValid() && !(duplicateLink == gNodes->SnapLinkIdx))
		{
			return false;
		}

		return true;
	}

	void UpdateClickInteraction(EditorContext& editor)
	{
		switch (editor.clickInteraction.Type)
		{
			case ClickInteractionType_BoxSelection: {
				editor.clickInteraction.BoxSelector.Rect.max =
				    ScreenToGridPosition(editor, gNodes->mousePosition);

				Rect boxRect = editor.clickInteraction.BoxSelector.Rect;
				boxRect.min  = GridToScreenPosition(editor, boxRect.min);
				boxRect.max  = GridToScreenPosition(editor, boxRect.max);

				BoxSelectorUpdateSelection(editor, boxRect);

				const Color boxSelector        = gNodes->Style.colors[ColorVar_BoxSelector];
				const Color boxSelectorOutline = gNodes->Style.colors[ColorVar_BoxSelectorOutline];
				gNodes->CanvasDrawList->AddRectFilled(
				    boxRect.min, boxRect.max, boxSelector.ToPackedABGR());
				gNodes->CanvasDrawList->AddRect(
				    boxRect.min, boxRect.max, boxSelectorOutline.ToPackedABGR());

				if (gNodes->leftMouseReleased)
				{
					ImVector<i32>& depthStack         = editor.NodeDepthOrder;
					const ImVector<i32>& selectedIdxs = editor.SelectedNodeIndices;

					// Bump the selected node indices, in order, to the top of the depth stack.
					// NOTE: this algorithm has worst case time complexity of O(N^2), if the
					// node selection is ~ N (due to selected_idxs.contains()).

					if ((selectedIdxs.Size > 0) && (selectedIdxs.Size < depthStack.Size))
					{
						// The number of indices moved. Stop after selected_idxs.Size
						i32 numMoved = 0;
						for (i32 i = 0; i < depthStack.Size - selectedIdxs.Size; ++i)
						{
							for (i32 nodeIdx = depthStack[i]; selectedIdxs.contains(nodeIdx);
							     nodeIdx     = depthStack[i])
							{
								depthStack.erase(depthStack.begin() + static_cast<size_t>(i));
								depthStack.push_back(nodeIdx);
								++numMoved;
							}

							if (numMoved == selectedIdxs.Size)
							{
								break;
							}
						}
					}

					editor.clickInteraction.Type = ClickInteractionType_None;
				}
				break;
			}
			case ClickInteractionType_Node: {
				TranslateSelectedNodes(editor);

				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.Type = ClickInteractionType_None;
				}
				break;
			}
			case ClickInteractionType_Link: {
				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.Type = ClickInteractionType_None;
				}
				break;
			}
			case ClickInteractionType_LinkCreation: {
				const PinData& startPin =
				    editor.pins.Pool[editor.clickInteraction.LinkCreation.outputPinIdx];

				const OptionalIndex maybeDuplicateLinkIdx =
				    gNodes->HoveredPinIdx.IsValid() ? FindDuplicateLink(editor,
				        editor.clickInteraction.LinkCreation.outputPinIdx,
				        gNodes->HoveredPinIdx.Value())
				                                    : OptionalIndex();

				const bool shouldSnap = gNodes->HoveredPinIdx.IsValid()
				                     && ShouldLinkSnapToPin(editor, startPin,
				                         gNodes->HoveredPinIdx.Value(), maybeDuplicateLinkIdx);

				// If we created on snap and the hovered pin is empty or changed, then we need
				// signal that the link's state has changed.
				const bool snappingPinChanged =
				    editor.clickInteraction.LinkCreation.inputPinIdx.IsValid()
				    && !(gNodes->HoveredPinIdx == editor.clickInteraction.LinkCreation.inputPinIdx);

				// Detach the link that was created by this link event if it's no longer in snap
				// range
				if (snappingPinChanged && gNodes->SnapLinkIdx.IsValid())
				{
					BeginLinkDetach(editor, gNodes->SnapLinkIdx.Value(),
					    editor.clickInteraction.LinkCreation.inputPinIdx.Value());
				}

				const v2 startPos = GetScreenSpacePinCoordinates(editor, startPin);
				// If we are within the hover radius of a receiving pin, snap the link
				// endpoint to it
				const v2 endPos = shouldSnap ? GetScreenSpacePinCoordinates(
				                      editor, editor.pins.Pool[gNodes->HoveredPinIdx.Value()])
				                             : gNodes->mousePosition;

				const CubicBezier cubicBezier = MakeCubicBezier(
				    startPos, endPos, startPin.Type, gNodes->Style.linkLineSegmentsPerLength);

				gNodes->CanvasDrawList->AddBezierCubic(cubicBezier.p0, cubicBezier.p1,
				    cubicBezier.p2, cubicBezier.p3,
				    gNodes->Style.colors[ColorVar_Link].ToPackedABGR(), gNodes->Style.LinkThickness,
				    cubicBezier.numSegments);

				const bool linkCreationOnSnap =
				    gNodes->HoveredPinIdx.IsValid()
				    && (editor.pins.Pool[gNodes->HoveredPinIdx.Value()].Flags
				        & PinFlags_EnableLinkCreationOnSnap);

				if (!shouldSnap)
				{
					editor.clickInteraction.LinkCreation.inputPinIdx.Reset();
				}

				const bool createLink =
				    shouldSnap && (gNodes->leftMouseReleased || linkCreationOnSnap);

				if (createLink && !maybeDuplicateLinkIdx.IsValid())
				{
					// Avoid send OnLinkCreated() events every frame if the snap link is not
					// saved (only applies for EnableLinkCreationOnSnap)
					if (!gNodes->leftMouseReleased
					    && editor.clickInteraction.LinkCreation.inputPinIdx
					           == gNodes->HoveredPinIdx)
					{
						break;
					}

					gNodes->UIState |= UIState_LinkCreated;
					editor.clickInteraction.LinkCreation.inputPinIdx =
					    gNodes->HoveredPinIdx.Value();
				}

				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.Type = ClickInteractionType_None;
					if (!createLink)
					{
						gNodes->UIState |= UIState_LinkDropped;
					}
				}
				break;
			}
			case ClickInteractionType_Panning: {
				const bool dragging = gNodes->AltMouseDragging;

				if (dragging)
				{
					editor.Panning += v2{ImGui::GetIO().MouseDelta};
				}
				else
				{
					editor.clickInteraction.Type = ClickInteractionType_None;
				}
				break;
			}
			case ClickInteractionType_ImGuiItem: {
				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.Type = ClickInteractionType_None;
				}
			}
			case ClickInteractionType_None: break;
			default: assert(!"Unreachable code!"); break;
		}
	}

	void ResolveOccludedPins(const EditorContext& editor, ImVector<i32>& occludedPinIndices)
	{
		const ImVector<i32>& depthStack = editor.NodeDepthOrder;

		occludedPinIndices.resize(0);

		if (depthStack.Size < 2)
		{
			return;
		}

		// For each node in the depth stack
		for (i32 depthIdx = 0; depthIdx < (depthStack.Size - 1); ++depthIdx)
		{
			const NodeData& nodeBelow = editor.nodes.Pool[depthStack[depthIdx]];

			// Iterate over the rest of the depth stack to find nodes overlapping the pins
			for (i32 nextDepthIdx = depthIdx + 1; nextDepthIdx < depthStack.Size; ++nextDepthIdx)
			{
				const Rect& rectAbove = editor.nodes.Pool[depthStack[nextDepthIdx]].Rect;

				// Iterate over each pin
				for (i32 idx = 0; idx < nodeBelow.pinIndices.Size; ++idx)
				{
					const i32 pinIdx = nodeBelow.pinIndices[idx];
					const v2& pinPos = editor.pins.Pool[pinIdx].Pos;

					if (rectAbove.Contains(pinPos))
					{
						occludedPinIndices.push_back(pinIdx);
					}
				}
			}
		}
	}

	OptionalIndex ResolveHoveredPin(
	    const ObjectPool<PinData>& pins, const ImVector<i32>& occludedPinIndices)
	{
		float smallestDistance = FLT_MAX;
		OptionalIndex pinIdxWithSmallestDistance;

		const float hoverRadiusSqr = gNodes->Style.PinHoverRadius * gNodes->Style.PinHoverRadius;

		for (i32 idx = 0; idx < pins.Pool.Size(); ++idx)
		{
			if (!pins.InUse.IsSet(idx))
			{
				continue;
			}

			if (occludedPinIndices.contains(idx))
			{
				continue;
			}

			const v2& pinPos        = pins.Pool[idx].Pos;
			const float distanceSqr = ImLengthSqr(pinPos - gNodes->mousePosition);

			// TODO: gNodes->Style.PinHoverRadius needs to be copied i32o pin data and the
			// pin-local value used here. This is no longer called in
			// BeginPin/EndPin scope and the detected pin might have a different
			// hover radius than what the user had when calling BeginPin/EndPin.
			if (distanceSqr < hoverRadiusSqr && distanceSqr < smallestDistance)
			{
				smallestDistance           = distanceSqr;
				pinIdxWithSmallestDistance = idx;
			}
		}

		return pinIdxWithSmallestDistance;
	}

	OptionalIndex ResolveHoveredNode(const ImVector<i32>& depthStack)
	{
		if (gNodes->NodeIndicesOverlappingWithMouse.size() == 0)
		{
			return {};
		}

		if (gNodes->NodeIndicesOverlappingWithMouse.size() == 1)
		{
			return OptionalIndex(gNodes->NodeIndicesOverlappingWithMouse[0]);
		}

		i32 largestDepthIdx = -1;
		i32 nodeIdxOnTop    = -1;

		for (int nodeIdx : gNodes->NodeIndicesOverlappingWithMouse)
		{
			for (i32 depthIdx = 0; depthIdx < depthStack.size(); ++depthIdx)
			{
				if (depthStack[depthIdx] == nodeIdx && (depthIdx > largestDepthIdx))
				{
					largestDepthIdx = depthIdx;
					nodeIdxOnTop    = nodeIdx;
				}
			}
		}

		assert(nodeIdxOnTop != -1);
		return OptionalIndex(nodeIdxOnTop);
	}

	OptionalIndex ResolveHoveredLink(
	    const ObjectPool<LinkData>& links, const ObjectPool<PinData>& pins)
	{
		float smallestDistance = FLT_MAX;
		OptionalIndex linkIdxWithSmallestDistance;

		// There are two ways a link can be detected as "hovered".
		// 1. The link is within hover distance to the mouse. The closest such link is selected
		// as being hovered over.
		// 2. If the link is connected to the currently hovered pin.
		//
		// The latter is a requirement for link detaching with drag click to work, as both a
		// link and pin are required to be hovered over for the feature to work.

		for (i32 idx = 0; idx < links.Pool.Size(); ++idx)
		{
			if (!links.InUse.IsSet(idx))
			{
				continue;
			}

			const LinkData& link    = links.Pool[idx];
			const PinData& startPin = pins.Pool[link.outputPinIdx];
			const PinData& endPin   = pins.Pool[link.inputPinIdx];

			// If there is a hovered pin links can only be considered hovered if they use that
			// pin
			if (gNodes->HoveredPinIdx.IsValid())
			{
				if (gNodes->HoveredPinIdx == link.outputPinIdx
				    || gNodes->HoveredPinIdx == link.inputPinIdx)
				{
					return idx;
				}
				continue;
			}

			// TODO: the calculated CubicBeziers could be cached since we generate them again
			// when rendering the links

			const CubicBezier cubicBezier = MakeCubicBezier(
			    startPin.Pos, endPin.Pos, startPin.Type, gNodes->Style.linkLineSegmentsPerLength);

			// The distance test
			{
				const Rect linkRect = GetContainingRectForCubicBezier(cubicBezier);

				// First, do a simple bounding box test against the box containing the link
				// to see whether calculating the distance to the link is worth doing.
				if (linkRect.Contains(gNodes->mousePosition))
				{
					const float distance = GetDistanceToCubicBezier(
					    gNodes->mousePosition, cubicBezier, cubicBezier.numSegments);

					// TODO: gNodes->Style.LinkHoverDistance could be also copied i32o
					// LinkData, since we're not calling this function in the same scope as
					// Nodes::Link(). The rendered/detected link might have a different hover
					// distance than what the user had specified when calling Link()
					if (distance < gNodes->Style.LinkHoverDistance && distance < smallestDistance)
					{
						smallestDistance            = distance;
						linkIdxWithSmallestDistance = idx;
					}
				}
			}
		}

		return linkIdxWithSmallestDistance;
	}

	// [SECTION] render helpers

	Rect GetItemRect()
	{
		return {ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
	}

	v2 GetNodeTitleBarOrigin(const NodeData& node)
	{
		return node.Origin + node.LayoutStyle.Padding;
	}

	v2 GetNodeContentOrigin(const NodeData& node)
	{
		const v2 titleBarHeight =
		    v2(0.f, node.TitleBarContentRect.GetSize().y + 2.0f * node.LayoutStyle.Padding.y);
		return node.Origin + titleBarHeight + node.LayoutStyle.Padding;
	}

	Rect GetNodeTitleRect(const NodeData& node)
	{
		Rect expandedTitleRect = node.TitleBarContentRect;
		expandedTitleRect.Expand(node.LayoutStyle.Padding);

		return {expandedTitleRect.min, expandedTitleRect.min + v2(node.Rect.GetSize().x, 0.f)
		                                   + v2(0.f, expandedTitleRect.GetSize().y)};
	}

	void DrawGrid(EditorContext& editor, const v2& canvasSize)
	{
		const v2 offset   = editor.Panning;
		u32 lineColor     = gNodes->Style.colors[ColorVar_GridLine].ToPackedABGR();
		u32 lineColorPrim = gNodes->Style.colors[ColorVar_GridLinePrimary].ToPackedABGR();
		bool drawPrimary  = gNodes->Style.Flags & StyleFlags_GridLinesPrimary;

		for (float x = fmodf(offset.x, gNodes->Style.GridSpacing); x < canvasSize.x;
		     x += gNodes->Style.GridSpacing)
		{
			gNodes->CanvasDrawList->AddLine(EditorToScreenPosition(v2(x, 0.0f)),
			    EditorToScreenPosition(v2(x, canvasSize.y)),
			    offset.x - x == 0.f && drawPrimary ? lineColorPrim : lineColor);
		}

		for (float y = fmodf(offset.y, gNodes->Style.GridSpacing); y < canvasSize.y;
		     y += gNodes->Style.GridSpacing)
		{
			gNodes->CanvasDrawList->AddLine(EditorToScreenPosition(v2(0.0f, y)),
			    EditorToScreenPosition(v2(canvasSize.x, y)),
			    offset.y - y == 0.f && drawPrimary ? lineColorPrim : lineColor);
		}
	}

	struct QuadOffsets
	{
		v2 topLeft, bottomLeft, bottomRight, topRight;
	};

	QuadOffsets CalculateQuadOffsets(const float sideLength)
	{
		const float halfSide = 0.5f * sideLength;

		QuadOffsets offset;

		offset.topLeft     = v2(-halfSide, halfSide);
		offset.bottomLeft  = v2(-halfSide, -halfSide);
		offset.bottomRight = v2(halfSide, -halfSide);
		offset.topRight    = v2(halfSide, halfSide);

		return offset;
	}

	struct TriangleOffsets
	{
		v2 topLeft, bottomLeft, right;
	};

	TriangleOffsets CalculateTriangleOffsets(const float sideLength)
	{
		// Calculates the Vec2 offsets from an equilateral triangle's midpoint to
		// its vertices. Here is how the left_offset and right_offset are
		// calculated.
		//
		// For an equilateral triangle of side length s, the
		// triangle's height, h, is h = s * sqrt(3) / 2.
		//
		// The length from the base to the midpoint is (1 / 3) * h. The length from
		// the midpoint to the triangle vertex is (2 / 3) * h.
		const float sqrt3          = sqrtf(3.0f);
		const float leftOffset     = -0.1666666666667f * sqrt3 * sideLength;
		const float rightOffset    = 0.333333333333f * sqrt3 * sideLength;
		const float verticalOffset = 0.5f * sideLength;

		TriangleOffsets offset;
		offset.topLeft    = v2(leftOffset, verticalOffset);
		offset.bottomLeft = v2(leftOffset, -verticalOffset);
		offset.right      = v2(rightOffset, 0.f);

		return offset;
	}

	void DrawPinShape(const v2& pinPos, const PinData& pin, const Color pinColor)
	{
		static const i32 circleNumSegments = 8;

		switch (pin.Shape)
		{
			case PinShape_Circle: {
				gNodes->CanvasDrawList->AddCircle(pinPos, gNodes->Style.PinCircleRadius,
				    pinColor.ToPackedABGR(), circleNumSegments, gNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_CircleFilled: {
				gNodes->CanvasDrawList->AddCircleFilled(pinPos, gNodes->Style.PinCircleRadius,
				    pinColor.ToPackedABGR(), circleNumSegments);
			}
			break;
			case PinShape_Quad: {
				const QuadOffsets offset = CalculateQuadOffsets(gNodes->Style.PinQuadSideLength);
				gNodes->CanvasDrawList->AddQuad(pinPos + offset.topLeft, pinPos + offset.bottomLeft,
				    pinPos + offset.bottomRight, pinPos + offset.topRight, pinColor.ToPackedABGR(),
				    gNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_QuadFilled: {
				const QuadOffsets offset = CalculateQuadOffsets(gNodes->Style.PinQuadSideLength);
				gNodes->CanvasDrawList->AddQuadFilled(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.bottomRight,
				    pinPos + offset.topRight, pinColor.ToPackedABGR());
			}
			break;
			case PinShape_Triangle: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(gNodes->Style.PinTriangleSideLength);
				gNodes->CanvasDrawList->AddTriangle(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.right, pinColor.ToPackedABGR(),
				    // NOTE: for some weird reason, the line drawn by AddTriangle is
				    // much thinner than the lines drawn by AddCircle or AddQuad.
				    // Multiplying the line thickness by two seemed to solve the
				    // problem at a few different thickness values.
				    2.f * gNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_TriangleFilled: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(gNodes->Style.PinTriangleSideLength);
				gNodes->CanvasDrawList->AddTriangleFilled(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.right, pinColor.ToPackedABGR());
			}
			break;
			default: assert(!"Invalid PinShape value!"); break;
		}
	}

	void DrawPin(EditorContext& editor, const i32 pinIdx)
	{
		PinData& pin               = editor.pins.Pool[pinIdx];
		const Rect& parentNodeRect = editor.nodes.Pool[pin.ParentNodeIdx].Rect;

		pin.Pos = GetScreenSpacePinCoordinates(parentNodeRect, pin.rect, pin.Type);

		Color pinColor = pin.colorStyle.Background;
		if (gNodes->HoveredPinIdx == pinIdx)
		{
			pinColor = pin.colorStyle.Hovered;
		}

		DrawPinShape(pin.Pos, pin, pinColor);
	}

	void DrawNode(EditorContext& editor, const i32 nodeIdx)
	{
		const NodeData& node = editor.nodes.Pool[nodeIdx];
		ImGui::SetCursorPos(node.Origin + editor.Panning);

		const bool nodeHovered = gNodes->HoveredNodeIdx == nodeIdx
		                      && editor.clickInteraction.Type != ClickInteractionType_BoxSelection;

		Color nodeBackground     = node.colorStyle.Background;
		Color titlebarBackground = node.colorStyle.Titlebar;
		if (editor.SelectedNodeIndices.contains(nodeIdx))
		{
			nodeBackground     = node.colorStyle.BackgroundSelected;
			titlebarBackground = node.colorStyle.TitlebarSelected;
		}
		else if (nodeHovered)
		{
			nodeBackground     = node.colorStyle.BackgroundHovered;
			titlebarBackground = node.colorStyle.TitlebarHovered;
		}

		{
			// node base
			gNodes->CanvasDrawList->AddRectFilled(node.Rect.min, node.Rect.max,
			    nodeBackground.ToPackedABGR(), node.LayoutStyle.CornerRounding);

			// title bar:
			if (node.TitleBarContentRect.GetSize().y > 0.f)
			{
				Rect titleBarRect = GetNodeTitleRect(node);

				gNodes->CanvasDrawList->AddRectFilled(titleBarRect.min, titleBarRect.max,
				    titlebarBackground.ToPackedABGR(), node.LayoutStyle.CornerRounding,
				    ImDrawFlags_RoundCornersTop);
			}

			if ((gNodes->Style.Flags & StyleFlags_NodeOutline) != 0
			    && node.LayoutStyle.BorderThickness > 0.f)
			{
				float halfBorder = node.LayoutStyle.BorderThickness * 0.5f;
				v2 min           = node.Rect.min;
				min.x -= halfBorder;
				min.y -= halfBorder;
				v2 max = node.Rect.max;
				max.x += halfBorder;
				max.y += halfBorder;
				gNodes->CanvasDrawList->AddRect(min, max, node.colorStyle.Outline.ToPackedABGR(),
				    node.LayoutStyle.CornerRounding, ImDrawFlags_RoundCornersAll,
				    node.LayoutStyle.BorderThickness);
			}
		}

		for (i32 pinIndex : node.pinIndices)
		{
			DrawPin(editor, pinIndex);
		}

		if (nodeHovered)
		{
			gNodes->HoveredNodeIdx = nodeIdx;
		}
	}

	void DrawLink(EditorContext& editor, const i32 linkIdx)
	{
		const LinkData& link    = editor.Links.Pool[linkIdx];
		const PinData& startPin = editor.pins.Pool[link.outputPinIdx];
		const PinData& endPin   = editor.pins.Pool[link.inputPinIdx];

		const CubicBezier cubicBezier = MakeCubicBezier(
		    startPin.Pos, endPin.Pos, startPin.Type, gNodes->Style.linkLineSegmentsPerLength);

		const bool linkHovered = gNodes->HoveredLinkIdx == linkIdx
		                      && editor.clickInteraction.Type != ClickInteractionType_BoxSelection;

		if (linkHovered)
		{
			gNodes->HoveredLinkIdx = linkIdx;
		}

		// It's possible for a link to be deleted in begin_link_i32eraction. A user
		// may detach a link, resulting in the link wire snapping to the mouse
		// position.
		//
		// In other words, skip rendering the link if it was deleted.
		if (gNodes->DeletedLinkIdx == linkIdx)
		{
			return;
		}

		Color linkColor = link.colorStyle.Base;
		if (editor.SelectedLinkIndices.contains(linkIdx))
		{
			linkColor = link.colorStyle.Selected;
		}
		else if (linkHovered)
		{
			linkColor = link.colorStyle.Hovered;
		}

		gNodes->CanvasDrawList->AddBezierCubic(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
		    cubicBezier.p3, linkColor.ToPackedABGR(), gNodes->Style.LinkThickness,
		    cubicBezier.numSegments);
	}

	void BeginPin(const i32 id, const PinType type, const PinShape shape, const i32 nodeIdx)
	{
		// Make sure to call BeginNode() before calling
		// BeginPin()
		assert(gNodes->CurrentScope == Scope_Node);
		gNodes->CurrentScope = Scope_Pin;

		ImGui::BeginGroup();
		ImGui::PushID(id);

		EditorContext& editor = GetEditorContext();

		gNodes->CurrentPinId = id;

		const i32 pinIdx          = ObjectPoolFindOrCreateIndex(editor.pins, id);
		gNodes->CurrentPinIdx     = pinIdx;
		PinData& pin              = editor.pins.Pool[pinIdx];
		pin.Id                    = id;
		pin.ParentNodeIdx         = nodeIdx;
		pin.Type                  = type;
		pin.Shape                 = shape;
		pin.Flags                 = gNodes->CurrentPinFlags;
		pin.colorStyle.Background = gNodes->Style.colors[ColorVar_Pin];
		pin.colorStyle.Hovered    = gNodes->Style.colors[ColorVar_PinHovered];
	}

	void EndPin()
	{
		assert(gNodes->CurrentScope == Scope_Pin);
		gNodes->CurrentScope = Scope_Node;

		ImGui::PopID();
		ImGui::EndGroup();

		if (ImGui::IsItemActive())
		{
			gNodes->activePin   = true;
			gNodes->activePinId = gNodes->CurrentPinId;
		}

		EditorContext& editor = GetEditorContext();
		PinData& pin          = editor.pins.Pool[gNodes->CurrentPinIdx];
		NodeData& node        = editor.nodes.Pool[gNodes->CurrentNodeIdx];
		pin.rect              = GetItemRect();
		node.pinIndices.push_back(gNodes->CurrentPinIdx);
	}

	void Initialize(Context* context)
	{
		context->CanvasOriginScreenSpace = v2(0.0f, 0.0f);
		context->CanvasRectScreenSpace   = Rect(v2(0.f, 0.f), v2(0.f, 0.f));
		context->CurrentScope            = Scope_None;

		context->CurrentPinIdx  = INT_MAX;
		context->CurrentNodeIdx = INT_MAX;

		context->DefaultEditorCtx = EditorContextCreate();
		EditorContextSet(gNodes->DefaultEditorCtx);

		context->CurrentPinFlags = PinFlags_None;
		context->pinFlagStack.push_back(gNodes->CurrentPinFlags);

		StyleColorsDark();
	}

	void Shutdown(Context* ctx)
	{
		EditorContextFree(ctx->DefaultEditorCtx);
	}


	// [SECTION] selection helpers

	template<typename T>
	void SelectObject(const ObjectPool<T>& objects, ImVector<i32>& selectedIndices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		assert(idx >= 0);
		assert(selectedIndices.find(idx) == selectedIndices.end());
		selectedIndices.push_back(idx);
	}

	template<typename T>
	void ClearObjectSelection(
	    const ObjectPool<T>& objects, ImVector<i32>& selectedIndices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		assert(idx >= 0);
		assert(selectedIndices.find(idx) != selectedIndices.end());
		selectedIndices.find_erase_unsorted(idx);
	}

	template<typename T>
	bool IsObjectSelected(
	    const ObjectPool<T>& objects, ImVector<i32>& selectedIndices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		return selectedIndices.find(idx) != selectedIndices.end();
	}


	// [SECTION] API implementation

	Style::Style()
	    : GridSpacing(32.f)
	    , NodeCornerRounding(4.f)
	    , NodePadding(8.f, 8.f)
	    , NodeBorderThickness(1.f)
	    , LinkThickness(3.f)
	    , linkLineSegmentsPerLength(0.1f)
	    , LinkHoverDistance(10.f)
	    , PinCircleRadius(4.f)
	    , PinQuadSideLength(7.f)
	    , PinTriangleSideLength(9.5)
	    , PinLineThickness(1.f)
	    , PinHoverRadius(10.f)
	    , PinOffset(0.f)
	    , Flags(StyleFlags_NodeOutline | StyleFlags_GridLines)
	{}

	Context* CreateContext()
	{
		auto* ctx = IM_NEW(Context)();
		if (gNodes == nullptr)
			SetCurrentContext(ctx);
		Initialize(ctx);
		return ctx;
	}

	void DestroyContext(Context* ctx)
	{
		if (ctx == nullptr)
			ctx = gNodes;
		Shutdown(ctx);
		if (gNodes == ctx)
			SetCurrentContext(nullptr);
		IM_DELETE(ctx);
	}

	Context* GetCurrentContext()
	{
		return gNodes;
	}

	void SetCurrentContext(Context* ctx)
	{
		gNodes = ctx;
	}

	EditorContext* EditorContextCreate()
	{
		void* mem = ImGui::MemAlloc(sizeof(EditorContext));
		new (mem) EditorContext();
		return (EditorContext*)mem;
	}

	void EditorContextFree(EditorContext* ctx)
	{
		ctx->~EditorContext();
		ImGui::MemFree(ctx);
	}

	void EditorContextSet(EditorContext* ctx)
	{
		gNodes->EditorCtx = ctx;
	}

	v2 GetEditorContextPanning()
	{
		const EditorContext& editor = GetEditorContext();
		return editor.Panning;
	}

	void EditorContextResetPanning(const v2& pos)
	{
		EditorContext& editor = GetEditorContext();
		editor.Panning        = pos;
	}

	void EditorContextMoveToNode(const i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.nodes, nodeId);

		editor.Panning.x = -node.Origin.x;
		editor.Panning.y = -node.Origin.y;
	}

	void SetImGuiContext(ImGuiContext* ctx)
	{
		ImGui::SetCurrentContext(ctx);
	}

	IO& GetIO()
	{
		return gNodes->Io;
	}

	Style& GetStyle()
	{
		return gNodes->Style;
	}

	void StyleColorsDark()
	{
		gNodes->Style.colors[ColorVar_NodeBackground]         = Color::FromRGB(50, 50, 50);
		gNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(75, 75, 75);
		gNodes->Style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(75, 75, 75);
		gNodes->Style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		// title bar colors match ImGui's titlebg colors
		gNodes->Style.colors[ColorVar_TitleBar]         = Color::FromRGB(41, 74, 122);
		gNodes->Style.colors[ColorVar_TitleBarHovered]  = Color::FromRGB(66, 150, 250);
		gNodes->Style.colors[ColorVar_TitleBarSelected] = Color::FromRGB(66, 150, 250);
		// link colors match ImGui's slider grab colors
		gNodes->Style.colors[ColorVar_Link]         = Color::FromRGB(61, 133, 224, 200);
		gNodes->Style.colors[ColorVar_LinkHovered]  = Color::FromRGB(66, 150, 250);
		gNodes->Style.colors[ColorVar_LinkSelected] = Color::FromRGB(66, 150, 250);
		// pin colors match ImGui's button colors
		gNodes->Style.colors[ColorVar_Pin]        = Color::FromRGB(53, 150, 250, 180);
		gNodes->Style.colors[ColorVar_PinHovered] = Color::FromRGB(53, 150, 250);

		gNodes->Style.colors[ColorVar_BoxSelector]        = Color::FromRGB(61, 133, 224, 30);
		gNodes->Style.colors[ColorVar_BoxSelectorOutline] = Color::FromRGB(61, 133, 224, 150);

		gNodes->Style.colors[ColorVar_GridBackground] = Color::FromRGB(40, 40, 50, 200);
		gNodes->Style.colors[ColorVar_GridLine]       = Color::FromRGB(200, 200, 200, 40);

		gNodes->Style.colors[ColorVar_GridLinePrimary] = Color::FromRGB(240, 240, 240, 60);

		// minimap colors
		gNodes->Style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 150);
		gNodes->Style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->Style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->Style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered] = Color::FromRGB(200, 200, 200);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->Style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapLink]        = gNodes->Style.colors[ColorVar_Link];
		gNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->Style.colors[ColorVar_LinkSelected];
		gNodes->Style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void StyleColorsClassic()
	{
		gNodes->Style.colors[ColorVar_NodeBackground]         = Color::FromRGB(50, 50, 50);
		gNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(75, 75, 75);
		gNodes->Style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(75, 75, 75);
		gNodes->Style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		gNodes->Style.colors[ColorVar_TitleBar]               = Color::FromRGB(69, 69, 138);
		gNodes->Style.colors[ColorVar_TitleBarHovered]        = Color::FromRGB(82, 82, 161);
		gNodes->Style.colors[ColorVar_TitleBarSelected]       = Color::FromRGB(82, 82, 161);
		gNodes->Style.colors[ColorVar_Link]                   = Color::FromRGB(255, 255, 255, 100);
		gNodes->Style.colors[ColorVar_LinkHovered]            = Color::FromRGB(105, 99, 204, 153);
		gNodes->Style.colors[ColorVar_LinkSelected]           = Color::FromRGB(105, 99, 204, 153);
		gNodes->Style.colors[ColorVar_Pin]                    = Color::FromRGB(89, 102, 156, 170);
		gNodes->Style.colors[ColorVar_PinHovered]             = Color::FromRGB(102, 122, 179, 200);
		gNodes->Style.colors[ColorVar_BoxSelector]            = Color::FromRGB(82, 82, 161, 100);
		gNodes->Style.colors[ColorVar_BoxSelectorOutline]     = Color::FromRGB(82, 82, 161);
		gNodes->Style.colors[ColorVar_GridBackground]         = Color::FromRGB(40, 40, 50, 200);
		gNodes->Style.colors[ColorVar_GridLine]               = Color::FromRGB(200, 200, 200, 40);
		gNodes->Style.colors[ColorVar_GridLinePrimary]        = Color::FromRGB(240, 240, 240, 60);

		// minimap colors
		gNodes->Style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 100);
		gNodes->Style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->Style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->Style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    Color::FromRGB(200, 200, 240);
		gNodes->Style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapLink]        = gNodes->Style.colors[ColorVar_Link];
		gNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->Style.colors[ColorVar_LinkSelected];
		gNodes->Style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void StyleColorsLight()
	{
		gNodes->Style.colors[ColorVar_NodeBackground]         = Color::FromRGB(240, 240, 240);
		gNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(240, 240, 240);
		gNodes->Style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(240, 240, 240);
		gNodes->Style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		gNodes->Style.colors[ColorVar_TitleBar]               = Color::FromRGB(248, 248, 248);
		gNodes->Style.colors[ColorVar_TitleBarHovered]        = Color::FromRGB(209, 209, 209);
		gNodes->Style.colors[ColorVar_TitleBarSelected]       = Color::FromRGB(209, 209, 209);
		// original imgui values: 66, 150, 250
		gNodes->Style.colors[ColorVar_Link] = Color::FromRGB(66, 150, 250, 100);
		// original imgui values: 117, 138, 204
		gNodes->Style.colors[ColorVar_LinkHovered]  = Color::FromRGB(66, 150, 250, 242);
		gNodes->Style.colors[ColorVar_LinkSelected] = Color::FromRGB(66, 150, 250, 242);
		// original imgui values: 66, 150, 250
		gNodes->Style.colors[ColorVar_Pin]                = Color::FromRGB(66, 150, 250, 160);
		gNodes->Style.colors[ColorVar_PinHovered]         = Color::FromRGB(66, 150, 250);
		gNodes->Style.colors[ColorVar_BoxSelector]        = Color::FromRGB(90, 170, 250, 30);
		gNodes->Style.colors[ColorVar_BoxSelectorOutline] = Color::FromRGB(90, 170, 250, 150);
		gNodes->Style.colors[ColorVar_GridBackground]     = Color::FromRGB(225, 225, 225);
		gNodes->Style.colors[ColorVar_GridLine]           = Color::FromRGB(180, 180, 180, 100);
		gNodes->Style.colors[ColorVar_GridLinePrimary]    = Color::FromRGB(120, 120, 120, 100);

		// minimap colors
		gNodes->Style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 100);
		gNodes->Style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->Style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->Style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    Color::FromRGB(200, 200, 240);
		gNodes->Style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->Style.colors[ColorVar_MiniMapLink]        = gNodes->Style.colors[ColorVar_Link];
		gNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->Style.colors[ColorVar_LinkSelected];
		gNodes->Style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void BeginNodeEditor()
	{
		assert(gNodes->CurrentScope == Scope_None);
		gNodes->CurrentScope = Scope_Editor;

		// Reset state from previous pass

		EditorContext& editor    = GetEditorContext();
		editor.AutoPanningDelta  = v2(0, 0);
		editor.gridContentBounds = Rect(v2{FLT_MAX, FLT_MAX}, v2{FLT_MIN, FLT_MIN});
		editor.miniMap.enabled   = false;
		ObjectPoolReset(editor.nodes);
		ObjectPoolReset(editor.pins);
		ObjectPoolReset(editor.Links);

		gNodes->HoveredNodeIdx.Reset();
		gNodes->HoveredLinkIdx.Reset();
		gNodes->HoveredPinIdx.Reset();
		gNodes->DeletedLinkIdx.Reset();
		gNodes->SnapLinkIdx.Reset();

		gNodes->NodeIndicesOverlappingWithMouse.clear();

		gNodes->UIState = UIState_None;

		gNodes->mousePosition     = ImGui::GetIO().MousePos;
		gNodes->LeftMouseClicked  = ImGui::IsMouseClicked(0);
		gNodes->leftMouseReleased = ImGui::IsMouseReleased(0);
		gNodes->leftMouseDragging = ImGui::IsMouseDragging(0, 0.0f);
		gNodes->AltMouseClicked =
		    (gNodes->Io.emulateThreeButtonMouse.modifier != nullptr
		        && *gNodes->Io.emulateThreeButtonMouse.modifier && gNodes->LeftMouseClicked)
		    || ImGui::IsMouseClicked(gNodes->Io.AltMouseButton);
		gNodes->AltMouseDragging =
		    (gNodes->Io.emulateThreeButtonMouse.modifier != nullptr && gNodes->leftMouseDragging
		        && (*gNodes->Io.emulateThreeButtonMouse.modifier))
		    || ImGui::IsMouseDragging(gNodes->Io.AltMouseButton, 0.0f);
		gNodes->AltMouseScrollDelta    = ImGui::GetIO().MouseWheel;
		gNodes->multipleSelectModifier = (gNodes->Io.multipleSelectModifier.modifier != nullptr
		                                      ? *gNodes->Io.multipleSelectModifier.modifier
		                                      : ImGui::GetIO().KeyCtrl);
		gNodes->activePin              = false;

		ImGui::BeginGroup();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, v2(1.f, 1.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, v2(0.f, 0.f));
			ImGui::PushStyleColor(
			    ImGuiCol_ChildBg, gNodes->Style.colors[ColorVar_GridBackground].ToPackedABGR());
			ImGui::BeginChild("scrolling_region", v2(0.f, 0.f), true,
			    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove
			        | ImGuiWindowFlags_NoScrollWithMouse);
			gNodes->CanvasOriginScreenSpace = ImGui::GetCursorScreenPos();

			// NOTE: we have to fetch the canvas draw list *after* we call
			// BeginChild(), otherwise the ImGui UI elements are going to be
			// rendered i32o the parent window draw list.
			DrawListSet(ImGui::GetWindowDrawList());

			{
				const v2 canvasSize = ImGui::GetWindowSize();
				gNodes->CanvasRectScreenSpace =
				    Rect(EditorToScreenPosition(v2(0.f, 0.f)), EditorToScreenPosition(canvasSize));

				if (gNodes->Style.Flags & StyleFlags_GridLines)
				{
					DrawGrid(editor, canvasSize);
				}
			}
		}
	}

	void EndNodeEditor()
	{
		assert(gNodes->CurrentScope == Scope_Editor);
		gNodes->CurrentScope = Scope_None;

		EditorContext& editor = GetEditorContext();

		bool noGridContent = editor.gridContentBounds.IsInverted();
		if (noGridContent)
		{
			editor.gridContentBounds =
			    Rect{ScreenToGridPosition(editor, gNodes->CanvasRectScreenSpace.min),
			        ScreenToGridPosition(editor, gNodes->CanvasRectScreenSpace.max)};
		}

		// Detect ImGui i32eraction first, because it blocks i32eraction with the rest of the UI

		if (gNodes->LeftMouseClicked && ImGui::IsAnyItemActive())
		{
			editor.clickInteraction.Type = ClickInteractionType_ImGuiItem;
		}

		// Detect which UI element is being hovered over. Detection is done in a hierarchical
		// fashion, because a UI element being hovered excludes any other as being hovered over.

		// Don't do hovering detection for nodes/links/pins when i32eracting with the mini-map,
		// since its an *overlay* with its own i32eraction behavior and must have precedence during
		// mouse i32eraction.

		if ((editor.clickInteraction.Type == ClickInteractionType_None
		        || editor.clickInteraction.Type == ClickInteractionType_LinkCreation)
		    && IsMouseInCanvas() && !editor.miniMap.IsHovered())
		{
			// Pins needs some special care. We need to check the depth stack to see which pins are
			// being occluded by other nodes.
			ResolveOccludedPins(editor, gNodes->occludedPinIndices);

			gNodes->HoveredPinIdx = ResolveHoveredPin(editor.pins, gNodes->occludedPinIndices);

			if (!gNodes->HoveredPinIdx.IsValid())
			{
				// Resolve which node is actually on top and being hovered using the depth stack.
				gNodes->HoveredNodeIdx = ResolveHoveredNode(editor.NodeDepthOrder);
			}

			// We don't check for hovered pins here, because if we want to detach a link by clicking
			// and dragging, we need to have both a link and pin hovered.
			if (!gNodes->HoveredNodeIdx.IsValid())
			{
				gNodes->HoveredLinkIdx = ResolveHoveredLink(editor.Links, editor.pins);
			}
		}

		for (i32 nodeIdx = 0; nodeIdx < editor.nodes.Pool.Size(); ++nodeIdx)
		{
			if (editor.nodes.InUse.IsSet(nodeIdx))
			{
				DrawListActivateNodeBackground(nodeIdx);
				DrawNode(editor, nodeIdx);
			}
		}

		// In order to render the links underneath the nodes, we want to first select the bottom
		// draw channel.
		gNodes->CanvasDrawList->ChannelsSetCurrent(0);

		for (i32 linkIdx = 0; linkIdx < editor.Links.Pool.Size(); ++linkIdx)
		{
			if (editor.Links.InUse.IsSet(linkIdx))
			{
				DrawLink(editor, linkIdx);
			}
		}

		// Render the click interaction UI elements (partial links, box selector) on top of
		// everything else.

		DrawListAppendClickInteractionChannel();
		DrawListActivateClickInteractionChannel();

		if (editor.miniMap.IsActive())
		{
			editor.miniMap.CalculateLayout();
			editor.miniMap.Update();
		}

		// Handle node graph interaction

		if (!editor.miniMap.IsHovered())
		{
			if (gNodes->LeftMouseClicked && gNodes->HoveredLinkIdx.IsValid())
			{
				BeginLinkInteraction(editor, gNodes->HoveredLinkIdx.Value(), gNodes->HoveredPinIdx);
			}

			else if (gNodes->LeftMouseClicked && gNodes->HoveredPinIdx.IsValid())
			{
				BeginLinkCreation(editor, gNodes->HoveredPinIdx.Value());
			}

			else if (gNodes->LeftMouseClicked && gNodes->HoveredNodeIdx.IsValid())
			{
				BeginNodeSelection(editor, gNodes->HoveredNodeIdx.Value());
			}

			else if (gNodes->LeftMouseClicked || gNodes->leftMouseReleased
			         || gNodes->AltMouseClicked || gNodes->AltMouseScrollDelta != 0.f)
			{
				BeginCanvasInteraction(editor);
			}

			bool shouldAutoPan = editor.clickInteraction.Type == ClickInteractionType_BoxSelection
			                  || editor.clickInteraction.Type == ClickInteractionType_LinkCreation
			                  || editor.clickInteraction.Type == ClickInteractionType_Node;
			if (shouldAutoPan && !IsMouseInCanvas())
			{
				v2 mouse     = ImGui::GetMousePos();
				v2 center    = gNodes->CanvasRectScreenSpace.GetCenter();
				v2 direction = (center - mouse);
				direction    = direction * ImInvLength(direction, 0.0);

				editor.AutoPanningDelta =
				    direction * ImGui::GetIO().DeltaTime * gNodes->Io.AutoPanningSpeed;
				editor.Panning += editor.AutoPanningDelta;
			}
		}
		UpdateClickInteraction(editor);

		// At this point, draw commands have been issued for all nodes (and pins). Update the node
		// pool to detect unused node slots and remove those indices from the depth stack before
		// sorting the node draw commands by depth.
		ObjectPoolUpdate(editor.nodes);
		ObjectPoolUpdate(editor.pins);

		DrawListSortChannelsByDepth(editor.NodeDepthOrder);

		// After the links have been rendered, the link pool can be updated as well.
		ObjectPoolUpdate(editor.Links);

		// Finally, merge the draw channels
		gNodes->CanvasDrawList->ChannelsMerge();

		// pop style
		ImGui::EndChild();         // end scrolling region
		ImGui::PopStyleColor();    // pop child window background color
		ImGui::PopStyleVar();      // pop window padding
		ImGui::PopStyleVar();      // pop frame padding
		ImGui::EndGroup();
	}

	void BeginNode(const i32 nodeId)
	{
		// Remember to call BeginNodeEditor before calling BeginNode
		assert(gNodes->CurrentScope == Scope_Editor);
		gNodes->CurrentScope = Scope_Node;

		EditorContext& editor = GetEditorContext();

		const i32 nodeIdx      = ObjectPoolFindOrCreateIndex(editor.nodes, nodeId);
		gNodes->CurrentNodeIdx = nodeIdx;

		NodeData& node                     = editor.nodes.Pool[nodeIdx];
		node.colorStyle.Background         = gNodes->Style.colors[ColorVar_NodeBackground];
		node.colorStyle.BackgroundHovered  = gNodes->Style.colors[ColorVar_NodeBackgroundHovered];
		node.colorStyle.BackgroundSelected = gNodes->Style.colors[ColorVar_NodeBackgroundSelected];
		node.colorStyle.Outline            = gNodes->Style.colors[ColorVar_NodeOutline];
		node.colorStyle.Titlebar           = gNodes->Style.colors[ColorVar_TitleBar];
		node.colorStyle.TitlebarHovered    = gNodes->Style.colors[ColorVar_TitleBarHovered];
		node.colorStyle.TitlebarSelected   = gNodes->Style.colors[ColorVar_TitleBarSelected];
		node.LayoutStyle.CornerRounding    = gNodes->Style.NodeCornerRounding;
		node.LayoutStyle.Padding           = gNodes->Style.NodePadding;
		node.LayoutStyle.BorderThickness   = gNodes->Style.NodeBorderThickness;

		// ImGui::SetCursorPos sets the cursor position, local to the current widget
		// (in this case, the child object started in BeginNodeEditor). Use
		// ImGui::SetCursorScreenPos to set the screen space coordinates directly.
		ImGui::SetCursorPos(GridToEditorPosition(editor, GetNodeTitleBarOrigin(node)));

		DrawListAddNode(nodeIdx);
		DrawListActivateCurrentNodeForeground();

		ImGui::PushID(node.Id);
		ImGui::BeginGroup();
	}

	void EndNode()
	{
		assert(gNodes->CurrentScope == Scope_Node);
		gNodes->CurrentScope = Scope_Editor;

		EditorContext& editor = GetEditorContext();

		// The node's rectangle depends on the ImGui UI group size.
		ImGui::EndGroup();
		ImGui::PopID();

		NodeData& node = editor.nodes.Pool[gNodes->CurrentNodeIdx];
		node.Rect      = GetItemRect();
		node.Rect.Expand(node.LayoutStyle.Padding);

		editor.gridContentBounds.Add(node.Origin);
		editor.gridContentBounds.Add(node.Origin + node.Rect.GetSize());

		if (node.Rect.Contains(gNodes->mousePosition))
		{
			gNodes->NodeIndicesOverlappingWithMouse.push_back(gNodes->CurrentNodeIdx);
		}
	}

	v2 GetNodeDimensions(i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 nodeIdx     = ObjectPoolFind(editor.nodes, nodeId);
		assert(nodeIdx != -1);    // invalid nodeId
		const NodeData& node = editor.nodes.Pool[nodeIdx];
		return node.Rect.GetSize();
	}

	void BeginNodeTitleBar()
	{
		assert(gNodes->CurrentScope == Scope_Node);
		ImGui::BeginGroup();
	}

	void EndNodeTitleBar()
	{
		assert(gNodes->CurrentScope == Scope_Node);
		ImGui::EndGroup();

		EditorContext& editor    = GetEditorContext();
		NodeData& node           = editor.nodes.Pool[gNodes->CurrentNodeIdx];
		node.TitleBarContentRect = GetItemRect();

		Rect nodeTitleRect = GetNodeTitleRect(node);
		ImGui::ItemAdd({nodeTitleRect.min, nodeTitleRect.max}, ImGui::GetID("title_bar"));

		ImGui::SetCursorPos(GridToEditorPosition(editor, GetNodeContentOrigin(node)));
	}

	void BeginInput(const i32 id, const PinShape shape)
	{
		BeginPin(id, PinType::Input, shape, gNodes->CurrentNodeIdx);
	}

	void EndInput()
	{
		EndPin();
	}

	void BeginOutput(const i32 id, const PinShape shape)
	{
		BeginPin(id, PinType::Output, shape, gNodes->CurrentNodeIdx);
	}

	void EndOutput()
	{
		EndPin();
	}

	void PushPinFlag(const PinFlags flag)
	{
		gNodes->CurrentPinFlags |= flag;
		gNodes->pinFlagStack.push_back(gNodes->CurrentPinFlags);
	}

	void PopPinFlag()
	{
		// PopPinFlag called without a matching PushPinFlag!
		// The bottom value is always the default value, pushed in Initialize().
		assert(gNodes->pinFlagStack.size() > 1);

		gNodes->pinFlagStack.pop_back();
		gNodes->CurrentPinFlags = gNodes->pinFlagStack.back();
	}

	void Link(const i32 id, const i32 startPinId, const i32 endPinId)
	{
		assert(gNodes->CurrentScope == Scope_Editor);

		EditorContext& editor    = GetEditorContext();
		LinkData& link           = ObjectPoolFindOrCreateObject(editor.Links, id);
		link.Id                  = id;
		link.outputPinIdx        = ObjectPoolFindOrCreateIndex(editor.pins, startPinId);
		link.inputPinIdx         = ObjectPoolFindOrCreateIndex(editor.pins, endPinId);
		link.colorStyle.Base     = gNodes->Style.colors[ColorVar_Link];
		link.colorStyle.Hovered  = gNodes->Style.colors[ColorVar_LinkHovered];
		link.colorStyle.Selected = gNodes->Style.colors[ColorVar_LinkSelected];

		// Check if this link was created by the current link event
		if ((editor.clickInteraction.Type == ClickInteractionType_LinkCreation
		        && editor.pins.Pool[link.inputPinIdx].Flags & PinFlags_EnableLinkCreationOnSnap
		        && editor.clickInteraction.LinkCreation.outputPinIdx == link.outputPinIdx
		        && editor.clickInteraction.LinkCreation.inputPinIdx == link.inputPinIdx)
		    || (editor.clickInteraction.LinkCreation.outputPinIdx == link.inputPinIdx
		        && editor.clickInteraction.LinkCreation.inputPinIdx == link.outputPinIdx))
		{
			gNodes->SnapLinkIdx = ObjectPoolFindOrCreateIndex(editor.Links, id);
		}
	}

	void PushStyleColor(const ColorVar item, Color color)
	{
		gNodes->ColorModifierStack.push_back(ColElement(item, gNodes->Style.colors[item]));
		gNodes->Style.colors[item] = color;
	}

	void PopStyleColor(i32 count)
	{
		assert(gNodes->ColorModifierStack.size() >= count);
		while (count > 0)
		{
			const ColElement& elem          = gNodes->ColorModifierStack.back();
			gNodes->Style.colors[elem.item] = elem.color;
			gNodes->ColorModifierStack.pop_back();
			--count;
		}
	}

	struct StyleVarInfo
	{
		ImGuiDataType type;
		u32 count;
		u32 offset;
		void* GetVarPtr(Style* style) const
		{
			return (void*)((u8*)style + offset);
		}
	};

	static const StyleVarInfo gStyleVarInfo[] = {
  // StyleVar_GridSpacing
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, GridSpacing)              },
 // StyleVar_NodeCornerRounding
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, NodeCornerRounding)       },
 // StyleVar_NodePadding
	    {ImGuiDataType_Float, 2, (u32)IM_OFFSETOF(Style, NodePadding)              },
 // StyleVar_NodeBorderThickness
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, NodeBorderThickness)      },
 // StyleVar_LinkThickness
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, LinkThickness)            },
 // StyleVar_linkLineSegmentsPerLength
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, linkLineSegmentsPerLength)},
 // StyleVar_LinkHoverDistance
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, LinkHoverDistance)        },
 // StyleVar_PinCircleRadius
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinCircleRadius)          },
 // StyleVar_PinQuadSideLength
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinQuadSideLength)        },
 // StyleVar_PinTriangleSideLength
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinTriangleSideLength)    },
 // StyleVar_PinLineThickness
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinLineThickness)         },
 // StyleVar_PinHoverRadius
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinHoverRadius)           },
 // StyleVar_PinOffset
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinOffset)                },
 // StyleVar_MiniMapPadding
	    {ImGuiDataType_Float, 2, (u32)IM_OFFSETOF(Style, miniMapPadding)           },
 // StyleVar_MiniMapOffset
	    {ImGuiDataType_Float, 2, (u32)IM_OFFSETOF(Style, miniMapOffset)            },
	};

	static const StyleVarInfo* GetStyleVarInfo(StyleVar idx)
	{
		IM_ASSERT(idx >= 0 && idx < StyleVar_COUNT);
		IM_ASSERT(IM_ARRAYSIZE(gStyleVarInfo) == StyleVar_COUNT);
		return &gStyleVarInfo[idx];
	}

	void PushStyleVar(const StyleVar item, const float value)
	{
		const StyleVarInfo* varInfo = GetStyleVarInfo(item);
		if (varInfo->type == ImGuiDataType_Float && varInfo->count == 1)
		{
			float& styleVar = *(float*)varInfo->GetVarPtr(&gNodes->Style);
			gNodes->StyleModifierStack.push_back(StyleVarElement(item, styleVar));
			styleVar = value;
			return;
		}
		IM_ASSERT(0 && "Called PushStyleVar() float variant but variable is not a float!");
	}

	void PushStyleVar(const StyleVar item, const v2& value)
	{
		const StyleVarInfo* varInfo = GetStyleVarInfo(item);
		if (varInfo->type == ImGuiDataType_Float && varInfo->count == 2)
		{
			v2& styleVar = *(v2*)varInfo->GetVarPtr(&gNodes->Style);
			gNodes->StyleModifierStack.push_back(StyleVarElement(item, styleVar));
			styleVar = value;
			return;
		}
		IM_ASSERT(0 && "Called PushStyleVar() v2 variant but variable is not a v2!");
	}

	void PopStyleVar(i32 count)
	{
		while (count > 0)
		{
			assert(gNodes->StyleModifierStack.size() > 0);
			const StyleVarElement styleBackup = gNodes->StyleModifierStack.back();
			gNodes->StyleModifierStack.pop_back();
			const StyleVarInfo* varInfo = GetStyleVarInfo(styleBackup.item);
			void* styleVar              = varInfo->GetVarPtr(&gNodes->Style);
			if (varInfo->type == ImGuiDataType_Float && varInfo->count == 1)
			{
				((float*)styleVar)[0] = styleBackup.value[0];
			}
			else if (varInfo->type == ImGuiDataType_Float && varInfo->count == 2)
			{
				((float*)styleVar)[0] = styleBackup.value[0];
				((float*)styleVar)[1] = styleBackup.value[1];
			}
			count--;
		}
	}

	void SetNodeScreenSpacePos(const i32 nodeId, const v2& screenSpacePos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.nodes, nodeId);
		node.Origin           = ScreenToGridPosition(editor, screenSpacePos);
	}

	void SetNodeEditorSpacePos(const i32 nodeId, const v2& editorSpacePos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.nodes, nodeId);
		node.Origin           = EditorToGridPosition(editor, editorSpacePos);
	}

	void SetNodeGridSpacePos(const i32 nodeId, const v2& gridPos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.nodes, nodeId);
		node.Origin           = gridPos;
	}

	void SetNodeDraggable(const i32 nodeId, const bool draggable)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.nodes, nodeId);
		node.Draggable        = draggable;
	}

	v2 GetNodeScreenSpacePos(const i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 nodeIdx     = ObjectPoolFind(editor.nodes, nodeId);
		assert(nodeIdx != -1);
		NodeData& node = editor.nodes.Pool[nodeIdx];
		return GridToScreenPosition(editor, node.Origin);
	}

	v2 GetNodeEditorSpacePos(const i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 nodeIdx     = ObjectPoolFind(editor.nodes, nodeId);
		assert(nodeIdx != -1);
		NodeData& node = editor.nodes.Pool[nodeIdx];
		return GridToEditorPosition(editor, node.Origin);
	}

	v2 GetNodeGridSpacePos(const i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 nodeIdx     = ObjectPoolFind(editor.nodes, nodeId);
		assert(nodeIdx != -1);
		NodeData& node = editor.nodes.Pool[nodeIdx];
		return node.Origin;
	}

	bool IsEditorHovered()
	{
		return IsMouseInCanvas();
	}

	bool IsNodeHovered(i32* const nodeId)
	{
		assert(gNodes->CurrentScope != Scope_None);
		assert(nodeId != nullptr);

		const bool isHovered = gNodes->HoveredNodeIdx.IsValid();
		if (isHovered)
		{
			const EditorContext& editor = GetEditorContext();
			*nodeId                     = editor.nodes.Pool[gNodes->HoveredNodeIdx.Value()].Id;
		}
		return isHovered;
	}

	bool IsLinkHovered(i32* const linkId)
	{
		assert(gNodes->CurrentScope != Scope_None);
		assert(linkId != nullptr);

		const bool isHovered = gNodes->HoveredLinkIdx.IsValid();
		if (isHovered)
		{
			const EditorContext& editor = GetEditorContext();
			*linkId                     = editor.Links.Pool[gNodes->HoveredLinkIdx.Value()].Id;
		}
		return isHovered;
	}

	bool IsPinHovered(i32* const attr)
	{
		assert(gNodes->CurrentScope != Scope_None);
		assert(attr != nullptr);

		const bool isHovered = gNodes->HoveredPinIdx.IsValid();
		if (isHovered)
		{
			const EditorContext& editor = GetEditorContext();
			*attr                       = editor.pins.Pool[gNodes->HoveredPinIdx.Value()].Id;
		}
		return isHovered;
	}

	i32 NumSelectedNodes()
	{
		assert(gNodes->CurrentScope != Scope_None);
		const EditorContext& editor = GetEditorContext();
		return editor.SelectedNodeIndices.size();
	}

	i32 NumSelectedLinks()
	{
		assert(gNodes->CurrentScope != Scope_None);
		const EditorContext& editor = GetEditorContext();
		return editor.SelectedLinkIndices.size();
	}

	void GetSelectedNodes(i32* nodeIds)
	{
		assert(nodeIds != nullptr);

		const EditorContext& editor = GetEditorContext();
		for (i32 i = 0; i < editor.SelectedNodeIndices.size(); ++i)
		{
			const i32 nodeIdx = editor.SelectedNodeIndices[i];
			nodeIds[i]        = editor.nodes.Pool[nodeIdx].Id;
		}
	}

	void GetSelectedLinks(i32* linkIds)
	{
		assert(linkIds != nullptr);

		const EditorContext& editor = GetEditorContext();
		for (i32 i = 0; i < editor.SelectedLinkIndices.size(); ++i)
		{
			const i32 linkIdx = editor.SelectedLinkIndices[i];
			linkIds[i]        = editor.Links.Pool[linkIdx].Id;
		}
	}

	void ClearNodeSelection()
	{
		EditorContext& editor = GetEditorContext();
		editor.SelectedNodeIndices.clear();
	}

	void ClearNodeSelection(i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		ClearObjectSelection(editor.nodes, editor.SelectedNodeIndices, nodeId);
	}

	void ClearLinkSelection()
	{
		EditorContext& editor = GetEditorContext();
		editor.SelectedLinkIndices.clear();
	}

	void ClearLinkSelection(i32 linkId)
	{
		EditorContext& editor = GetEditorContext();
		ClearObjectSelection(editor.Links, editor.SelectedLinkIndices, linkId);
	}

	void SelectNode(i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		SelectObject(editor.nodes, editor.SelectedNodeIndices, nodeId);
	}

	void SelectLink(i32 linkId)
	{
		EditorContext& editor = GetEditorContext();
		SelectObject(editor.Links, editor.SelectedLinkIndices, linkId);
	}

	bool IsNodeSelected(i32 nodeId)
	{
		EditorContext& editor = GetEditorContext();
		return IsObjectSelected(editor.nodes, editor.SelectedNodeIndices, nodeId);
	}

	bool IsLinkSelected(i32 linkId)
	{
		EditorContext& editor = GetEditorContext();
		return IsObjectSelected(editor.Links, editor.SelectedLinkIndices, linkId);
	}

	bool IsPinActive()
	{
		assert((gNodes->CurrentScope & Scope_Node) != 0);

		if (!gNodes->activePin)
		{
			return false;
		}

		return gNodes->activePinId == gNodes->CurrentPinId;
	}

	bool IsAnyPinActive(i32* const pinId)
	{
		assert((gNodes->CurrentScope & (Scope_Node | Scope_Pin)) == 0);

		if (!gNodes->activePin)
		{
			return false;
		}

		if (pinId != nullptr)
		{
			*pinId = gNodes->activePinId;
		}

		return true;
	}

	bool IsLinkStarted(i32* const startedAtId)
	{
		// Call this function after EndNodeEditor()!
		assert(gNodes->CurrentScope != Scope_None);
		assert(startedAtId != nullptr);

		const bool isStarted = (gNodes->UIState & UIState_LinkStarted) != 0;
		if (isStarted)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 pinIdx            = editor.clickInteraction.LinkCreation.outputPinIdx;
			*startedAtId                = editor.pins.Pool[pinIdx].Id;
		}

		return isStarted;
	}

	bool IsLinkDropped(i32* const startedAtId, const bool includingDetachedLinks)
	{
		// Call this function after EndNodeEditor()!
		assert(gNodes->CurrentScope != Scope_None);

		const EditorContext& editor = GetEditorContext();

		const bool linkDropped =
		    (gNodes->UIState & UIState_LinkDropped) != 0
		    && (includingDetachedLinks
		        || editor.clickInteraction.LinkCreation.Type != LinkCreationType_FromDetach);

		if (linkDropped && startedAtId)
		{
			const i32 pinIdx = editor.clickInteraction.LinkCreation.outputPinIdx;
			*startedAtId     = editor.pins.Pool[pinIdx].Id;
		}

		return linkDropped;
	}

	bool IsLinkCreated(
	    i32* const startedAtPinId, i32* const endedAtPinId, bool* const createdFromSnap)
	{
		assert(gNodes->CurrentScope != Scope_None);
		assert(startedAtPinId != nullptr);
		assert(endedAtPinId != nullptr);

		const bool isCreated = (gNodes->UIState & UIState_LinkCreated) != 0;

		if (isCreated)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 startIdx          = editor.clickInteraction.LinkCreation.outputPinIdx;
			const i32 endIdx            = editor.clickInteraction.LinkCreation.inputPinIdx.Value();
			const PinData& startPin     = editor.pins.Pool[startIdx];
			const PinData& endPin       = editor.pins.Pool[endIdx];

			if (startPin.Type == PinType::Output)
			{
				*startedAtPinId = startPin.Id;
				*endedAtPinId   = endPin.Id;
			}
			else
			{
				*startedAtPinId = endPin.Id;
				*endedAtPinId   = startPin.Id;
			}

			if (createdFromSnap)
			{
				*createdFromSnap =
				    editor.clickInteraction.Type == ClickInteractionType_LinkCreation;
			}
		}

		return isCreated;
	}

	bool IsLinkCreated(i32* startedAtNodeId, i32* startedAtPinId, i32* endedAtNodeId,
	    i32* endedAtPinId, bool* createdFromSnap)
	{
		assert(gNodes->CurrentScope != Scope_None);
		assert(startedAtNodeId != nullptr);
		assert(startedAtPinId != nullptr);
		assert(endedAtNodeId != nullptr);
		assert(endedAtPinId != nullptr);

		const bool isCreated = (gNodes->UIState & UIState_LinkCreated) != 0;

		if (isCreated)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 startIdx          = editor.clickInteraction.LinkCreation.outputPinIdx;
			const i32 endIdx            = editor.clickInteraction.LinkCreation.inputPinIdx.Value();
			const PinData& startPin     = editor.pins.Pool[startIdx];
			const NodeData& startNode   = editor.nodes.Pool[startPin.ParentNodeIdx];
			const PinData& endPin       = editor.pins.Pool[endIdx];
			const NodeData& endNode     = editor.nodes.Pool[endPin.ParentNodeIdx];

			if (startPin.Type == PinType::Output)
			{
				*startedAtPinId  = startPin.Id;
				*startedAtNodeId = startNode.Id;
				*endedAtPinId    = endPin.Id;
				*endedAtNodeId   = endNode.Id;
			}
			else
			{
				*startedAtPinId  = endPin.Id;
				*startedAtNodeId = endNode.Id;
				*endedAtPinId    = startPin.Id;
				*endedAtNodeId   = startNode.Id;
			}

			if (createdFromSnap)
			{
				*createdFromSnap =
				    editor.clickInteraction.Type == ClickInteractionType_LinkCreation;
			}
		}

		return isCreated;
	}

	bool IsLinkDestroyed(i32* const linkId)
	{
		assert(gNodes->CurrentScope != Scope_None);

		const bool linkDestroyed = gNodes->DeletedLinkIdx.IsValid();
		if (linkDestroyed)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 linkIdx           = gNodes->DeletedLinkIdx.Value();
			*linkId                     = editor.Links.Pool[linkIdx].Id;
		}

		return linkDestroyed;
	}
}    // namespace Rift::Nodes
