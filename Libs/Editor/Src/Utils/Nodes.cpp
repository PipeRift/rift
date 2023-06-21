// Copyright 2015-2023 Piperift - All rights reserved

// the structure of this file:
//
// [SECTION] bezier curve helpers
// [SECTION] draw list helper
// [SECTION] ui state logic
// [SECTION] render helpers
// [SECTION] API implementation

#include "Utils/Nodes.h"

#include "Utils/NodesInternal.h"
#include "Utils/NodesMiniMap.h"

#include <Pipe/Core/Checks.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Math/Bezier.h>

#include <cassert>
#include <cstring>    // strlen, strncmp
#include <new>


namespace rift::Nodes
{
	Context* gNodes = nullptr;

	EditorContext& GetEditorContext()
	{
		// No editor context was set! Did you forget to call Nodes::CreateContext()?
		Check(gNodes->EditorCtx != nullptr);
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
			v2 pCurrent = p::EvaluateCubicBezier(cb.p0, cb.p1, cb.p2, cb.p3, tStep * i);
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
		const v2 min = v2(math::Min(cb.p0.x, cb.p3.x), math::Min(cb.p0.y, cb.p3.y));
		const v2 max = v2(math::Max(cb.p0.x, cb.p3.x), math::Max(cb.p0.y, cb.p3.y));

		const float hoverDistance = gNodes->style.LinkHoverDistance;

		Rect rect(min, max);
		rect.Merge(cb.p1);
		rect.Merge(cb.p2);
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
		v2 current = p::EvaluateCubicBezier(
		    cubicBezier.p0, cubicBezier.p1, cubicBezier.p2, cubicBezier.p3, 0.f);

		const float dt = 1.0f / cubicBezier.numSegments;
		for (i32 s = 0; s < cubicBezier.numSegments; ++s)
		{
			const v2 next = p::EvaluateCubicBezier(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
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
			    MakeCubicBezier(start, end, startType, gNodes->style.linkLineSegmentsPerLength);
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
		return v - gNodes->CanvasOriginScreenSpace - editor.panning;
	}

	v2 GridToScreenPosition(const EditorContext& editor, const v2& v)
	{
		return v + gNodes->CanvasOriginScreenSpace + editor.panning;
	}

	v2 GridToEditorPosition(const EditorContext& editor, const v2& v)
	{
		return v + editor.panning;
	}

	v2 EditorToGridPosition(const EditorContext& editor, const v2& v)
	{
		return v - editor.panning;
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
		gNodes->nodeSubmissionOrder.clear();
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

	void DrawListAddNode(AST::Id nodeId)
	{
		gNodes->NodeIdxToSubmissionIdx.SetInt(
		    static_cast<ImGuiID>(GetIdIndex(nodeId)), gNodes->nodeSubmissionOrder.Size);
		gNodes->nodeSubmissionOrder.push_back(nodeId);
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
		    DrawListSubmissionIdxToForegroundChannelIdx(gNodes->nodeSubmissionOrder.Size - 1);
		gNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    gNodes->CanvasDrawList, foregroundChannelIdx);
	}

	void DrawListActivateNodeBackground(AST::Id nodeId)
	{
		const i32 submissionIdx =
		    gNodes->NodeIdxToSubmissionIdx.GetInt(static_cast<ImGuiID>(GetIdIndex(nodeId)), -1);
		// There is a discrepancy in the submitted node count and the rendered node count! Did
		// you call one of the following functions
		// * MoveToNode
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

	void DrawListSortChannelsByDepth(const TArray<AST::Id>& nodeDepthOrder)
	{
		if (gNodes->NodeIdxToSubmissionIdx.Data.Size < 2)
		{
			return;
		}

		assert(nodeDepthOrder.Size() == gNodes->nodeSubmissionOrder.Size);

		i32 startIdx = nodeDepthOrder.Size() - 1;
		while (nodeDepthOrder[startIdx] == gNodes->nodeSubmissionOrder[startIdx])
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
			const AST::Id nodeId = nodeDepthOrder[depthIdx];

			// Find the current index of the nodeIdx in the submission order array
			i32 submissionIdx = -1;
			for (i32 i = 0; i < gNodes->nodeSubmissionOrder.Size; ++i)
			{
				if (gNodes->nodeSubmissionOrder[i] == nodeId)
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
				ImSwap(gNodes->nodeSubmissionOrder[j], gNodes->nodeSubmissionOrder[j + 1]);
			}
		}
	}

	// [SECTION] ui state logic

	v2 GetScreenSpacePinCoordinates(const Rect& nodeRect, const PinType type, const Rect& pinRect)
	{
		const float x = type == PinType::Input ? (nodeRect.min.x - gNodes->style.PinOffset)
		                                       : (nodeRect.max.x + gNodes->style.PinOffset);
		return {x, 0.5f * (pinRect.min.y + pinRect.max.y)};
	}

	v2 GetScreenSpacePinCoordinates(const EditorContext& editor, PinType type, const PinData& pin)
	{
		const Rect& parentNodeRect = editor.nodes.Get(pin.parentNodeId).rect;
		return GetScreenSpacePinCoordinates(parentNodeRect, type, pin.rect);
	}

	bool IsMouseInCanvas()
	{
		// This flag should be true either when hovering or clicking something in the canvas.
		const bool isWindowHoveredOrFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();

		return isWindowHoveredOrFocused
		    && gNodes->CanvasRectScreenSpace.Contains(ImGui::GetMousePos());
	}

	void BeginNodeSelection(EditorContext& editor, AST::Id nodeId)
	{
		// Don't start selecting a node if we are e.g. already creating and dragging
		// a new link! New link creation can happen when the mouse is clicked over
		// a node, but within the hover radius of a pin.
		if (editor.clickInteraction.type != ClickInteractionType_None)
		{
			return;
		}

		editor.clickInteraction.type = ClickInteractionType_Node;
		// If the node is not already contained in the selection, then we want only
		// the i32eraction node to be selected, effective immediately.
		//
		// If the multiple selection modifier is active, we want to add this node
		// to the current list of selected nodes.
		//
		// Otherwise, we want to allow for the possibility of multiple nodes to be
		// moved at once.
		if (!editor.selectedNodeIds.Contains(nodeId))
		{
			editor.selectedLinkIndices.clear();
			if (!gNodes->multipleSelectModifier)
				editor.selectedNodeIds.Clear();
			editor.selectedNodeIds.Add(nodeId);

			// Ensure that individually selected nodes get rendered on top
			editor.nodes.PushToTheFront(nodeId);
		}
		// Deselect a previously-selected node
		else if (gNodes->multipleSelectModifier)
		{
			editor.selectedNodeIds.Remove(nodeId);

			// Don't allow dragging after deselecting
			editor.clickInteraction.type = ClickInteractionType_None;
		}

		// To support snapping of multiple nodes, we need to store the offset of
		// each node in the selection to the origin of the dragged node.
		const v2 refOrigin = editor.nodes.Get(nodeId).Origin;
		editor.PrimaryNodeOffset =
		    refOrigin + gNodes->CanvasOriginScreenSpace + editor.panning - gNodes->mousePosition;

		editor.SelectedNodeOrigins.clear();
		for (AST::Id id : editor.selectedNodeIds)
		{
			const v2 nodeOrigin = editor.nodes.Get(id).Origin - refOrigin;
			editor.SelectedNodeOrigins.push_back(nodeOrigin);
		}
	}

	void BeginLinkSelection(EditorContext& editor, const i32 linkIdx)
	{
		editor.clickInteraction.type = ClickInteractionType_Link;
		// When a link is selected, clear all other selections, and insert the link
		// as the sole selection.
		editor.selectedNodeIds.Clear();
		editor.selectedLinkIndices.clear();
		editor.selectedLinkIndices.push_back(linkIdx);
	}

	void BeginLinkDetach(EditorContext& editor, i32 linkIdx, PinIdx detachPinIdx)
	{
		const LinkData& link         = editor.links.pool[linkIdx];
		ClickInteractionState& state = editor.clickInteraction;
		state.type                   = ClickInteractionType_LinkCreation;
		const bool pinIsOutput       = detachPinIdx.type == PinType::Output;
		state.linkCreation.inputIdx  = pinIsOutput ? link.inputIdx : NO_INDEX;
		state.linkCreation.outputIdx = !pinIsOutput ? link.outputIdx : NO_INDEX;
		gNodes->DeletedLinkIdx       = linkIdx;
	}

	void BeginLinkCreation(EditorContext& editor, PinIdx hoveredPinIdx)
	{
		if (!gNodes->canCreateLinks)
		{
			return;
		}

		editor.clickInteraction.type = ClickInteractionType_LinkCreation;

		const bool pinIsOutput = hoveredPinIdx.type == PinType::Output;
		editor.clickInteraction.linkCreation.outputIdx =
		    pinIsOutput ? hoveredPinIdx.index : NO_INDEX;
		editor.clickInteraction.linkCreation.inputIdx =
		    !pinIsOutput ? hoveredPinIdx.index : NO_INDEX;
		editor.clickInteraction.linkCreation.type = LinkCreationType_Standard;
		gNodes->UIState |= UIState_LinkStarted;
	}

	void BeginLinkInteraction(
	    EditorContext& editor, const i32 linkIdx, const PinIdx pin = PinIdx::Invalid())
	{
		// Check if we are clicking the link with the modifier pressed.
		// This will in a link detach via clicking.
		const bool modifierPressed = gNodes->io.linkDetachWithModifierClick.modifier
		                          && *gNodes->io.linkDetachWithModifierClick.modifier;

		if (modifierPressed)
		{
			const LinkData& link     = editor.links.pool[linkIdx];
			const PinData& outputPin = editor.outputs.pool[link.outputIdx];
			const PinData& inputPin  = editor.inputs.pool[link.inputIdx];
			const v2& mousePos       = gNodes->mousePosition;
			const float distToStart  = ImLengthSqr(outputPin.position - mousePos);
			const float distToEnd    = ImLengthSqr(inputPin.position - mousePos);
			const PinIdx closestPin  = distToStart < distToEnd
			                             ? PinIdx{link.outputIdx, PinType::Output}
			                             : PinIdx{link.inputIdx, PinType::Input};

			editor.clickInteraction.type = ClickInteractionType_LinkCreation;
			BeginLinkDetach(editor, linkIdx, closestPin);
			editor.clickInteraction.linkCreation.type = LinkCreationType_FromDetach;
		}
		else if (pin)
		{
			const i32 hoveredPinFlags = editor.GetPinData(pin).Flags;

			// Check the 'click and drag to detach' case.
			if (hoveredPinFlags & PinFlags_EnableLinkDetachWithDragClick)
			{
				BeginLinkDetach(editor, linkIdx, pin);
				editor.clickInteraction.linkCreation.type = LinkCreationType_FromDetach;
			}
			else
			{
				BeginLinkCreation(editor, pin);
			}
		}
		else
		{
			BeginLinkSelection(editor, linkIdx);
		}
	}

	static bool IsMiniMapHovered();

	void BeginCanvasInteraction(EditorContext& editor)
	{
		const bool anyUIElementHovered = !IsNone(gNodes->hoveredNodeId)
		                              || gNodes->HoveredLinkIdx.IsValid() || gNodes->HoveredPinIdx
		                              || ImGui::IsAnyItemHovered();

		const bool mouseNotInCanvas = !IsMouseInCanvas();

		if (editor.clickInteraction.type != ClickInteractionType_None || anyUIElementHovered
		    || mouseNotInCanvas)
		{
			return;
		}

		const bool startedPanning = gNodes->altMouseClicked;

		if (startedPanning)
		{
			editor.clickInteraction.type = ClickInteractionType_Panning;
		}
		else if (gNodes->leftMouseClicked)
		{
			editor.clickInteraction.type = ClickInteractionType_BoxSelection;
			editor.clickInteraction.boxSelector.rect.min =
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

		editor.selectedNodeIds.Clear();

		// Test for overlap against node rectangles

		for (AST::Id nodeId : editor.nodes)
		{
			NodeData& node = editor.nodes.Get(nodeId);
			if (boxRect.Overlaps(node.rect))
			{
				editor.selectedNodeIds.Add(nodeId);
			}
		}

		// Update link selection

		editor.selectedLinkIndices.clear();

		// Test for overlap against links

		for (i32 linkIdx = 0; linkIdx < editor.links.pool.Size(); ++linkIdx)
		{
			if (editor.links.inUse.IsSet(linkIdx))
			{
				const LinkData& link = editor.links.pool[linkIdx];

				const PinData& outputPin  = editor.outputs.pool[link.outputIdx];
				const PinData& inputPin   = editor.inputs.pool[link.inputIdx];
				const Rect& nodeStartRect = editor.nodes[outputPin.parentNodeId].rect;
				const Rect& nodeEndRect   = editor.nodes[inputPin.parentNodeId].rect;

				const v2 start =
				    GetScreenSpacePinCoordinates(nodeStartRect, PinType::Output, outputPin.rect);
				const v2 end =
				    GetScreenSpacePinCoordinates(nodeEndRect, PinType::Input, inputPin.rect);

				// Test
				if (RectangleOverlapsLink(boxRect, start, end, PinType::Output))
				{
					editor.selectedLinkIndices.push_back(linkIdx);
				}
			}
		}
	}

	v2 SnapOriginToGrid(v2 origin)
	{
		if ((gNodes->style.Flags & StyleFlags_GridSnapping)
		    || ((gNodes->style.Flags & StyleFlags_GridSnappingOnRelease)
		        && gNodes->leftMouseReleased))
		{
			const float spacing  = gNodes->style.GridSpacing;
			const float spacing2 = spacing / 2.0f;
			float modx           = fmodf(fabsf(origin.x) + spacing2, spacing) - spacing2;
			float mody           = fmodf(fabsf(origin.y) + spacing2, spacing) - spacing2;
			origin.x += (origin.x < 0.f) ? modx : -modx;
			origin.y += (origin.y < 0.f) ? mody : -mody;
		}

		return origin;
	}

	OptionalIndex FindDuplicateLink(
	    const EditorContext& editor, const PinIdx pinA, const PinIdx pinB)
	{
		if (!pinA || !pinB || pinA.type == pinB.type)
		{
			// Pins must be valid and be one input and one output
			return {};
		}

		Id outputPin = pinA.index;
		Id inputPin  = pinB.index;
		if (pinB.type == PinType::Output)
		{
			Swap(outputPin, inputPin);
		}

		for (i32 i = 0; i < editor.links.pool.Size(); ++i)
		{
			if (editor.links.inUse.IsSet(i))
			{
				continue;
			}

			const LinkData& link = editor.links.pool[i];
			if (outputPin == link.outputIdx && inputPin == link.inputIdx)
			{
				return {i};
			}
		}
		return {};
	}

	bool CanLinkSnapToPin(const EditorContext& editor, PinIdx originPin, PinIdx targetPin,
	    OptionalIndex duplicateLink)
	{
		if (originPin.type == targetPin.type)
		{
			return false;    // The end pin must be of a different type
		}

		const PinData& originData = editor.GetPinData(originPin);
		const PinData& targetData = editor.GetPinData(targetPin);

		// The end pin must be in a different node
		if (originData.parentNodeId == targetData.parentNodeId)
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

	void TranslateSelectedNodes(EditorContext& editor)
	{
		if (gNodes->leftMouseDragging || gNodes->leftMouseReleased)
		{
			const v2 origin =
			    SnapOriginToGrid(gNodes->mousePosition - gNodes->CanvasOriginScreenSpace
			                     - editor.panning + editor.PrimaryNodeOffset);
			for (i32 i = 0; i < editor.selectedNodeIds.Size(); ++i)
			{
				const v2 nodeRel     = editor.SelectedNodeOrigins[i];
				const AST::Id nodeId = editor.selectedNodeIds[i];
				NodeData& node       = editor.nodes[nodeId];
				if (node.Draggable)
				{
					// node.Origin += io.MouseDelta - editor.AutoPanningDelta;
					node.Origin = origin + nodeRel;
				}
			}
		}

		if (gNodes->leftMouseReleased)
		{
			editor.clickInteraction.type = ClickInteractionType_None;
		}
	}

	void UpdateBoxSelection(EditorContext& editor)
	{
		editor.clickInteraction.boxSelector.rect.max =
		    ScreenToGridPosition(editor, gNodes->mousePosition);

		Rect boxRect = editor.clickInteraction.boxSelector.rect;
		boxRect.min  = GridToScreenPosition(editor, boxRect.min);
		boxRect.max  = GridToScreenPosition(editor, boxRect.max);

		BoxSelectorUpdateSelection(editor, boxRect);

		const Color boxSelector        = gNodes->style.colors[ColorVar_BoxSelector];
		const Color boxSelectorOutline = gNodes->style.colors[ColorVar_BoxSelectorOutline];
		gNodes->CanvasDrawList->AddRectFilled(boxRect.min, boxRect.max, boxSelector.ToPackedABGR());
		gNodes->CanvasDrawList->AddRect(
		    boxRect.min, boxRect.max, boxSelectorOutline.ToPackedABGR());

		if (gNodes->leftMouseReleased)
		{
			TArray<AST::Id>& depthStack        = editor.nodes.depthOrder;
			const TArray<AST::Id>& selectedIds = editor.selectedNodeIds;

			// Bump the selected node indices, in order, to the top of the depth stack.
			// NOTE: this algorithm has worst case time complexity of O(N^2), if the
			// node selection is ~ N (due to selected_ids.contains()).

			if ((selectedIds.Size() > 0) && (selectedIds.Size() < depthStack.Size()))
			{
				// The number of indices moved. Stop after selected_ids.Size
				i32 numMoved = 0;
				for (i32 i = 0; i < depthStack.Size() - selectedIds.Size(); ++i)
				{
					for (AST::Id nodeId = depthStack[i]; selectedIds.Contains(nodeId);
					     nodeId         = depthStack[i])
					{
						depthStack.RemoveAt(i, false);
						depthStack.Add(nodeId);
						++numMoved;
					}

					if (numMoved == selectedIds.Size())
					{
						break;
					}
				}
			}
			editor.clickInteraction.type = ClickInteractionType_None;
		}
	}

	void UpdateLinkCreation(EditorContext& editor)
	{
		const PinIdx outputIdx = PinIdx::Output(editor.clickInteraction.linkCreation.outputIdx);
		const PinIdx inputIdx  = PinIdx::Input(editor.clickInteraction.linkCreation.inputIdx);

		const PinIdx startIdx       = outputIdx ? outputIdx : inputIdx;
		const PinIdx endIdx         = outputIdx ? inputIdx : outputIdx;
		const PinData& startPinData = editor.GetPinData(startIdx);

		OptionalIndex maybeDuplicateLinkIdx;
		if (gNodes->HoveredPinIdx)
		{
			maybeDuplicateLinkIdx = FindDuplicateLink(editor, startIdx, gNodes->HoveredPinIdx);
		}

		const bool shouldSnap =
		    gNodes->HoveredPinIdx
		    && CanLinkSnapToPin(editor, startIdx, gNodes->HoveredPinIdx, maybeDuplicateLinkIdx);

		// If we created on snap and the hovered pin is empty or changed, then we need
		// signal that the link's state has changed.
		const bool snappingPinChanged = endIdx && !(gNodes->HoveredPinIdx == endIdx);

		// Detach the link that was created by this link event if it's no longer in snap
		// range
		if (snappingPinChanged && gNodes->SnapLinkIdx.IsValid())
		{
			BeginLinkDetach(editor, gNodes->SnapLinkIdx.Value(), endIdx);
		}

		const v2 startPos = GetScreenSpacePinCoordinates(editor, startIdx.type, startPinData);
		// If we are within the hover radius of a receiving pin, snap the link
		// endpoint to it
		const v2 endPos = shouldSnap ? GetScreenSpacePinCoordinates(editor,
		                      gNodes->HoveredPinIdx.type, editor.GetPinData(gNodes->HoveredPinIdx))
		                             : gNodes->mousePosition;

		const CubicBezier cubicBezier = MakeCubicBezier(
		    startPos, endPos, startIdx.type, gNodes->style.linkLineSegmentsPerLength);

		gNodes->CanvasDrawList->AddBezierCubic(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
		    cubicBezier.p3, gNodes->style.colors[ColorVar_Link].ToPackedABGR(),
		    gNodes->style.LinkThickness, cubicBezier.numSegments);

		const bool linkCreationOnSnap =
		    gNodes->HoveredPinIdx
		    && (editor.GetPinData(gNodes->HoveredPinIdx).Flags & PinFlags_EnableLinkCreationOnSnap);

		if (!shouldSnap)
		{
			if (startIdx.type == PinType::Input)
			{
				editor.clickInteraction.linkCreation.outputIdx = NO_INDEX;
			}
			else
			{
				editor.clickInteraction.linkCreation.inputIdx = NO_INDEX;
			}
		}

		const bool createLink = shouldSnap && (gNodes->leftMouseReleased || linkCreationOnSnap);

		if (createLink && !maybeDuplicateLinkIdx.IsValid())
		{
			// Avoid send IsLinkCreated() events every frame if the snap link is not
			// saved (only applies for EnableLinkCreationOnSnap)
			if (!gNodes->leftMouseReleased && endIdx == gNodes->HoveredPinIdx
			    && !gNodes->canCreateLinks)
			{
				return;
			}

			gNodes->UIState |= UIState_LinkCreated;
			if (gNodes->HoveredPinIdx.type == PinType::Output)
			{
				editor.clickInteraction.linkCreation.outputIdx = gNodes->HoveredPinIdx.index;
			}
			else
			{
				editor.clickInteraction.linkCreation.inputIdx = gNodes->HoveredPinIdx.index;
			}
		}

		if (gNodes->leftMouseReleased)
		{
			editor.clickInteraction.type = ClickInteractionType_None;
			if (!createLink)
			{
				gNodes->UIState |= UIState_LinkDropped;
			}
		}
	}

	void UpdatePanning(EditorContext& editor)
	{
		const bool dragging = gNodes->altMouseDragging;

		if (dragging)
		{
			editor.panning += v2{ImGui::GetIO().MouseDelta};
		}
		else
		{
			editor.clickInteraction.type = ClickInteractionType_None;
		}
	}

	void UpdateClickInteraction(EditorContext& editor)
	{
		switch (editor.clickInteraction.type)
		{
			case ClickInteractionType_BoxSelection: UpdateBoxSelection(editor); break;
			case ClickInteractionType_Node: TranslateSelectedNodes(editor); break;
			case ClickInteractionType_Link:
				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.type = ClickInteractionType_None;
				}
				break;
			case ClickInteractionType_LinkCreation: UpdateLinkCreation(editor); break;
			case ClickInteractionType_Panning: UpdatePanning(editor); break;
			case ClickInteractionType_ImGuiItem: {
				if (gNodes->leftMouseReleased)
				{
					editor.clickInteraction.type = ClickInteractionType_None;
				}
			}
			case ClickInteractionType_None:
			default: break;
		}
	}

	void ResolveOccludedPins(const EditorContext& editor, ImVector<PinIdx>& occludedPinIndices)
	{
		const TArray<AST::Id>& depthStack = editor.nodes.depthOrder;

		occludedPinIndices.resize(0);

		if (depthStack.Size() < 2)
		{
			return;
		}

		// For each node in the depth stack
		for (i32 i = 0; i < (depthStack.Size() - 1); ++i)
		{
			const NodeData& nodeBelow = editor.nodes[depthStack[i]];

			// Iterate over the rest of the depth stack to find nodes overlapping the pins
			for (i32 e = i + 1; e < depthStack.Size(); ++e)
			{
				const Rect& rectAbove = editor.nodes[depthStack[e]].rect;

				// Iterate over each pin
				for (i32 i = 0; i < nodeBelow.inputs.Size; ++i)
				{
					const i32 pinIdx = nodeBelow.inputs[i];
					const v2& pinPos = editor.inputs.pool[pinIdx].position;

					if (rectAbove.Contains(pinPos))
					{
						occludedPinIndices.push_back({pinIdx, PinType::Input});
					}
				}
				for (i32 i = 0; i < nodeBelow.outputs.Size; ++i)
				{
					const i32 pinIdx = nodeBelow.outputs[i];
					const v2& pinPos = editor.outputs.pool[pinIdx].position;

					if (rectAbove.Contains(pinPos))
					{
						occludedPinIndices.push_back({pinIdx, PinType::Output});
					}
				}
			}
		}
	}

	PinIdx ResolveHoveredPin(
	    const ObjectPool<PinData>& pins, PinType type, const ImVector<PinIdx>& occludedPinIndices)
	{
		float smallestDistance         = FLT_MAX;
		i32 pinIdxWithSmallestDistance = NO_INDEX;

		const float hoverRadiusSqr = gNodes->style.PinHoverRadius * gNodes->style.PinHoverRadius;

		for (i32 idx = 0; idx < pins.pool.Size(); ++idx)
		{
			if (!pins.inUse.IsSet(idx))
			{
				continue;
			}

			if (occludedPinIndices.contains({idx, type}))
			{
				continue;
			}

			const v2& pinPos        = pins.pool[idx].position;
			const float distanceSqr = (pinPos - gNodes->mousePosition).LengthSquared();

			// TODO: gNodes->style.PinHoverRadius needs to be copied i32o pin data and the
			// pin-local value used here. This is no longer called in
			// BeginPin/EndPin scope and the detected pin might have a different
			// hover radius than what the user had when calling BeginPin/EndPin.
			if (distanceSqr < hoverRadiusSqr && distanceSqr < smallestDistance)
			{
				smallestDistance           = distanceSqr;
				pinIdxWithSmallestDistance = idx;
			}
		}

		return {pinIdxWithSmallestDistance, type};
	}

	AST::Id ResolveHoveredNode(const TArray<AST::Id>& depthStack)
	{
		if (gNodes->nodeIdsOverlappingWithMouse.size() == 0)
		{
			return AST::NoId;
		}

		if (gNodes->nodeIdsOverlappingWithMouse.size() == 1)
		{
			return gNodes->nodeIdsOverlappingWithMouse[0];
		}

		i32 largestDepthIdx = -1;
		AST::Id nodeIdOnTop = AST::NoId;

		for (AST::Id nodeId : gNodes->nodeIdsOverlappingWithMouse)
		{
			for (i32 depthIdx = 0; depthIdx < depthStack.Size(); ++depthIdx)
			{
				if (depthStack[depthIdx] == nodeId && (depthIdx > largestDepthIdx))
				{
					largestDepthIdx = depthIdx;
					nodeIdOnTop     = nodeId;
				}
			}
		}

		assert(nodeIdOnTop != AST::NoId);
		return nodeIdOnTop;
	}

	OptionalIndex ResolveHoveredLink(const ObjectPool<LinkData>& links,
	    const ObjectPool<PinData>& outputs, const ObjectPool<PinData>& inputs)
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

		for (i32 idx = 0; idx < links.pool.Size(); ++idx)
		{
			if (!links.inUse.IsSet(idx))
			{
				continue;
			}

			const LinkData& link     = links.pool[idx];
			const PinData& outputPin = outputs.pool[link.outputIdx];
			const PinData& inputPin  = inputs.pool[link.inputIdx];

			// If there is a hovered pin links can only be considered hovered if they use that
			// pin
			if (gNodes->HoveredPinIdx)
			{
				if (gNodes->HoveredPinIdx == PinIdx::Output(link.outputIdx)
				    || gNodes->HoveredPinIdx == PinIdx::Input(link.inputIdx))
				{
					return idx;
				}
				continue;
			}

			// TODO: the calculated CubicBeziers could be cached since we generate them again
			// when rendering the links

			const CubicBezier cubicBezier = MakeCubicBezier(outputPin.position, inputPin.position,
			    PinType::Output, gNodes->style.linkLineSegmentsPerLength);

			// The distance test
			{
				const Rect linkRect = GetContainingRectForCubicBezier(cubicBezier);

				// First, do a simple bounding box test against the box containing the link
				// to see whether calculating the distance to the link is worth doing.
				if (linkRect.Contains(gNodes->mousePosition))
				{
					const float distance = GetDistanceToCubicBezier(
					    gNodes->mousePosition, cubicBezier, cubicBezier.numSegments);

					// TODO: gNodes->style.LinkHoverDistance could be also copied i32o
					// LinkData, since we're not calling this function in the same scope as
					// Nodes::Link(). The rendered/detected link might have a different hover
					// distance than what the user had specified when calling Link()
					if (distance < gNodes->style.LinkHoverDistance && distance < smallestDistance)
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

		return {expandedTitleRect.min, expandedTitleRect.min + v2(node.rect.GetSize().x, 0.f)
		                                   + v2(0.f, expandedTitleRect.GetSize().y)};
	}

	void DrawGrid(EditorContext& editor, const v2& canvasSize)
	{
		const v2 offset   = editor.panning;
		u32 lineColor     = gNodes->style.colors[ColorVar_GridLine].ToPackedABGR();
		u32 lineColorPrim = gNodes->style.colors[ColorVar_GridLinePrimary].ToPackedABGR();
		bool drawPrimary  = gNodes->style.Flags & StyleFlags_GridLinesPrimary;

		for (float x = fmodf(offset.x, gNodes->style.GridSpacing); x < canvasSize.x;
		     x += gNodes->style.GridSpacing)
		{
			gNodes->CanvasDrawList->AddLine(EditorToScreenPosition(v2(x, 0.0f)),
			    EditorToScreenPosition(v2(x, canvasSize.y)),
			    offset.x - x == 0.f && drawPrimary ? lineColorPrim : lineColor);
		}

		for (float y = fmodf(offset.y, gNodes->style.GridSpacing); y < canvasSize.y;
		     y += gNodes->style.GridSpacing)
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
				gNodes->CanvasDrawList->AddCircle(pinPos, gNodes->style.PinCircleRadius,
				    pinColor.ToPackedABGR(), circleNumSegments, gNodes->style.PinLineThickness);
			}
			break;
			case PinShape_CircleFilled: {
				gNodes->CanvasDrawList->AddCircleFilled(pinPos, gNodes->style.PinCircleRadius,
				    pinColor.ToPackedABGR(), circleNumSegments);
			}
			break;
			case PinShape_Quad: {
				const QuadOffsets offset = CalculateQuadOffsets(gNodes->style.PinQuadSideLength);
				gNodes->CanvasDrawList->AddQuad(pinPos + offset.topLeft, pinPos + offset.bottomLeft,
				    pinPos + offset.bottomRight, pinPos + offset.topRight, pinColor.ToPackedABGR(),
				    gNodes->style.PinLineThickness);
			}
			break;
			case PinShape_QuadFilled: {
				const QuadOffsets offset = CalculateQuadOffsets(gNodes->style.PinQuadSideLength);
				gNodes->CanvasDrawList->AddQuadFilled(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.bottomRight,
				    pinPos + offset.topRight, pinColor.ToPackedABGR());
			}
			break;
			case PinShape_Triangle: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(gNodes->style.PinTriangleSideLength);
				gNodes->CanvasDrawList->AddTriangle(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.right, pinColor.ToPackedABGR(),
				    // NOTE: for some weird reason, the line drawn by AddTriangle is
				    // much thinner than the lines drawn by AddCircle or AddQuad.
				    // Multiplying the line thickness by two seemed to solve the
				    // problem at a few different thickness values.
				    2.f * gNodes->style.PinLineThickness);
			}
			break;
			case PinShape_TriangleFilled: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(gNodes->style.PinTriangleSideLength);
				gNodes->CanvasDrawList->AddTriangleFilled(pinPos + offset.topLeft,
				    pinPos + offset.bottomLeft, pinPos + offset.right, pinColor.ToPackedABGR());
			}
			break;
			case PinShape_Diamond: {
				const float halfSide = 0.5f * gNodes->style.PinDiamondSideLength;
				gNodes->CanvasDrawList->AddQuad(pinPos + v2{0.f, halfSide},
				    pinPos + v2{halfSide, 0.f}, pinPos + v2{0.f, -halfSide},
				    pinPos + v2{-halfSide, 0.f}, pinColor.ToPackedABGR(),
				    gNodes->style.PinLineThickness);
			}
			break;
			case PinShape_DiamondFilled: {
				const float halfSide = 0.5f * gNodes->style.PinDiamondSideLength;
				gNodes->CanvasDrawList->AddQuadFilled(pinPos + v2{0.f, halfSide},
				    pinPos + v2{halfSide, 0.f}, pinPos + v2{0.f, -halfSide},
				    pinPos + v2{-halfSide, 0.f}, pinColor.ToPackedABGR());
			}
			break;
			default: assert(!"Invalid PinShape value!"); break;
		}
	}

	void DrawPin(EditorContext& editor, const PinIdx pin)
	{
		PinData& pinData           = editor.GetPinData(pin);
		const Rect& parentNodeRect = editor.nodes[pinData.parentNodeId].rect;

		pinData.position = GetScreenSpacePinCoordinates(parentNodeRect, pin.type, pinData.rect);

		Color pinColor = pinData.colorStyle.Background;
		if (gNodes->HoveredPinIdx == pin)
		{
			pinColor = pinData.colorStyle.Hovered;
		}

		DrawPinShape(pinData.position, pinData, pinColor);
	}

	void DrawNode(EditorContext& editor, const AST::Id nodeId)
	{
		const NodeData& node = editor.nodes[nodeId];
		ImGui::SetCursorPos(node.Origin + editor.panning);

		const bool nodeHovered = gNodes->hoveredNodeId == nodeId
		                      && editor.clickInteraction.type != ClickInteractionType_BoxSelection;

		Color nodeBackground     = node.colorStyle.Background;
		Color titlebarBackground = node.colorStyle.Titlebar;
		if (editor.selectedNodeIds.Contains(nodeId))
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
			gNodes->CanvasDrawList->AddRectFilled(node.rect.min, node.rect.max,
			    nodeBackground.ToPackedABGR(), node.LayoutStyle.CornerRounding);

			// title bar:
			if (node.TitleBarContentRect.GetSize().y > 0.f)
			{
				Rect titleBarRect = GetNodeTitleRect(node);

				gNodes->CanvasDrawList->AddRectFilled(titleBarRect.min, titleBarRect.max,
				    titlebarBackground.ToPackedABGR(), node.LayoutStyle.CornerRounding,
				    ImDrawFlags_RoundCornersTop);
			}

			if ((gNodes->style.Flags & StyleFlags_NodeOutline) != 0
			    && node.LayoutStyle.BorderThickness > 0.f)
			{
				float halfBorder = node.LayoutStyle.BorderThickness * 0.5f;
				v2 min           = node.rect.min;
				min.x -= halfBorder;
				min.y -= halfBorder;
				v2 max = node.rect.max;
				max.x += halfBorder;
				max.y += halfBorder;
				gNodes->CanvasDrawList->AddRect(min, max, node.colorStyle.Outline.ToPackedABGR(),
				    node.LayoutStyle.CornerRounding, ImDrawFlags_RoundCornersAll,
				    node.LayoutStyle.BorderThickness);
			}
		}

		for (i32 pinIndex : node.inputs)
		{
			DrawPin(editor, {pinIndex, PinType::Input});
		}
		for (i32 pinIndex : node.outputs)
		{
			DrawPin(editor, {pinIndex, PinType::Output});
		}

		if (nodeHovered)
		{
			gNodes->hoveredNodeId = nodeId;
		}
	}

	void DrawLink(EditorContext& editor, const i32 linkIdx)
	{
		const LinkData& link    = editor.links.pool[linkIdx];
		const PinData& startPin = editor.outputs.pool[link.outputIdx];
		const PinData& endPin   = editor.inputs.pool[link.inputIdx];

		const CubicBezier cubicBezier = MakeCubicBezier(startPin.position, endPin.position,
		    PinType::Output, gNodes->style.linkLineSegmentsPerLength);

		const bool linkHovered = gNodes->HoveredLinkIdx == linkIdx
		                      && editor.clickInteraction.type != ClickInteractionType_BoxSelection;

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
		if (editor.selectedLinkIndices.contains(linkIdx))
		{
			linkColor = link.colorStyle.Selected;
		}
		else if (linkHovered)
		{
			linkColor = link.colorStyle.Hovered;
		}

		gNodes->CanvasDrawList->AddBezierCubic(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
		    cubicBezier.p3, linkColor.ToPackedABGR(), gNodes->style.LinkThickness,
		    cubicBezier.numSegments);
	}

	void BeginPin(const i32 id, const PinType type, const PinShape shape, AST::Id nodeId)
	{
		// Make sure to call BeginNode() before calling
		// BeginPin()
		assert(gNodes->currentScope == Scope::Node);
		gNodes->currentScope = Scope::Pin;

		ImGui::BeginGroup();
		ImGui::PushID(id);

		EditorContext& editor = GetEditorContext();

		gNodes->CurrentPinId = id;

		auto& pool                = editor.GetPinPool(type);
		const i32 pinIdx          = ObjectPoolFindOrCreateIndex(pool, id);
		gNodes->CurrentPinIdx     = {pinIdx, type};
		PinData& pin              = pool.pool[pinIdx];
		pin.id                    = id;
		pin.parentNodeId          = nodeId;
		pin.Shape                 = shape;
		pin.Flags                 = gNodes->CurrentPinFlags;
		pin.colorStyle.Background = gNodes->style.colors[ColorVar_Pin];
		pin.colorStyle.Hovered    = gNodes->style.colors[ColorVar_PinHovered];
	}

	void EndPin()
	{
		assert(gNodes->currentScope == Scope::Pin);
		gNodes->currentScope = Scope::Node;

		ImGui::PopID();
		ImGui::EndGroup();

		if (ImGui::IsItemActive())
		{
			gNodes->activePin   = true;
			gNodes->activePinId = gNodes->CurrentPinId;
		}

		EditorContext& editor = GetEditorContext();
		PinData& pin          = editor.GetPinData(gNodes->CurrentPinIdx);
		NodeData& node        = editor.nodes.Get(gNodes->currentNodeId);
		pin.rect              = GetItemRect();

		switch (gNodes->CurrentPinIdx.type)
		{
			case PinType::Input: node.inputs.push_back(gNodes->CurrentPinIdx.index); break;
			case PinType::Output: node.outputs.push_back(gNodes->CurrentPinIdx.index);
		}
	}

	void Initialize(Context* context)
	{
		context->CanvasOriginScreenSpace = v2(0.0f, 0.0f);
		context->CanvasRectScreenSpace   = Rect(v2(0.f, 0.f), v2(0.f, 0.f));
		context->currentScope            = Scope::None;

		context->CurrentPinIdx = PinIdx::Invalid();
		context->currentNodeId = AST::NoId;

		context->DefaultEditorCtx = EditorContextCreate();
		SetEditorContext(gNodes->DefaultEditorCtx);

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
	bool IsObjectSelectedByIdx(
	    const ObjectPool<T>& objects, const ImVector<i32>& selectedIndices, const i32 idx)
	{
		return selectedIndices.find(idx) != selectedIndices.end();
	}

	template<typename T>
	bool IsObjectSelected(
	    const ObjectPool<T>& objects, const ImVector<i32>& selectedIndices, const Id id)
	{
		return IsObjectSelectedByIdx(objects, selectedIndices, ObjectPoolFind(objects, id));
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
	    , PinTriangleSideLength(9.5f)
	    , PinDiamondSideLength(7.f)
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

	void SetEditorContext(EditorContext* ctx)
	{
		gNodes->EditorCtx = ctx;
	}

	v2 GetPanning()
	{
		const EditorContext& editor = GetEditorContext();
		return editor.panning;
	}

	void ResetPanning(const v2& pos)
	{
		EditorContext& editor = GetEditorContext();
		editor.panning        = pos;
	}

	void MoveToNode(AST::Id nodeId, v2 offset)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = editor.nodes.Get(nodeId);

		editor.panning.x = -node.Origin.x;
		editor.panning.y = -node.Origin.y;
		editor.panning += offset;
	}

	void SetImGuiContext(ImGuiContext* ctx)
	{
		ImGui::SetCurrentContext(ctx);
	}

	IO& GetIO()
	{
		return gNodes->io;
	}

	Style& GetStyle()
	{
		return gNodes->style;
	}

	void StyleColorsDark()
	{
		gNodes->style.colors[ColorVar_NodeBackground]         = Color::FromRGB(50, 50, 50);
		gNodes->style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(75, 75, 75);
		gNodes->style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(75, 75, 75);
		gNodes->style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		// title bar colors match ImGui's titlebg colors
		gNodes->style.colors[ColorVar_TitleBar]         = Color::FromRGB(41, 74, 122);
		gNodes->style.colors[ColorVar_TitleBarHovered]  = Color::FromRGB(66, 150, 250);
		gNodes->style.colors[ColorVar_TitleBarSelected] = Color::FromRGB(66, 150, 250);
		// link colors match ImGui's slider grab colors
		gNodes->style.colors[ColorVar_Link]         = Color::FromRGB(61, 133, 224, 200);
		gNodes->style.colors[ColorVar_LinkHovered]  = Color::FromRGB(66, 150, 250);
		gNodes->style.colors[ColorVar_LinkSelected] = Color::FromRGB(66, 150, 250);
		// pin colors match ImGui's button colors
		gNodes->style.colors[ColorVar_Pin]        = Color::FromRGB(53, 150, 250, 180);
		gNodes->style.colors[ColorVar_PinHovered] = Color::FromRGB(53, 150, 250);

		gNodes->style.colors[ColorVar_BoxSelector]        = Color::FromRGB(61, 133, 224, 30);
		gNodes->style.colors[ColorVar_BoxSelectorOutline] = Color::FromRGB(61, 133, 224, 150);

		gNodes->style.colors[ColorVar_GridBackground] = Color::FromRGB(40, 40, 50, 200);
		gNodes->style.colors[ColorVar_GridLine]       = Color::FromRGB(200, 200, 200, 40);

		gNodes->style.colors[ColorVar_GridLinePrimary] = Color::FromRGB(240, 240, 240, 60);

		// minimap colors
		gNodes->style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 150);
		gNodes->style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundHovered] = Color::FromRGB(200, 200, 200);
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapLink]        = gNodes->style.colors[ColorVar_Link];
		gNodes->style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->style.colors[ColorVar_LinkSelected];
		gNodes->style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void StyleColorsClassic()
	{
		gNodes->style.colors[ColorVar_NodeBackground]         = Color::FromRGB(50, 50, 50);
		gNodes->style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(75, 75, 75);
		gNodes->style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(75, 75, 75);
		gNodes->style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		gNodes->style.colors[ColorVar_TitleBar]               = Color::FromRGB(69, 69, 138);
		gNodes->style.colors[ColorVar_TitleBarHovered]        = Color::FromRGB(82, 82, 161);
		gNodes->style.colors[ColorVar_TitleBarSelected]       = Color::FromRGB(82, 82, 161);
		gNodes->style.colors[ColorVar_Link]                   = Color::FromRGB(255, 255, 255, 100);
		gNodes->style.colors[ColorVar_LinkHovered]            = Color::FromRGB(105, 99, 204, 153);
		gNodes->style.colors[ColorVar_LinkSelected]           = Color::FromRGB(105, 99, 204, 153);
		gNodes->style.colors[ColorVar_Pin]                    = Color::FromRGB(89, 102, 156, 170);
		gNodes->style.colors[ColorVar_PinHovered]             = Color::FromRGB(102, 122, 179, 200);
		gNodes->style.colors[ColorVar_BoxSelector]            = Color::FromRGB(82, 82, 161, 100);
		gNodes->style.colors[ColorVar_BoxSelectorOutline]     = Color::FromRGB(82, 82, 161);
		gNodes->style.colors[ColorVar_GridBackground]         = Color::FromRGB(40, 40, 50, 200);
		gNodes->style.colors[ColorVar_GridLine]               = Color::FromRGB(200, 200, 200, 40);
		gNodes->style.colors[ColorVar_GridLinePrimary]        = Color::FromRGB(240, 240, 240, 60);

		// minimap colors
		gNodes->style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 100);
		gNodes->style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    Color::FromRGB(200, 200, 240);
		gNodes->style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapLink]        = gNodes->style.colors[ColorVar_Link];
		gNodes->style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->style.colors[ColorVar_LinkSelected];
		gNodes->style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void StyleColorsLight()
	{
		gNodes->style.colors[ColorVar_NodeBackground]         = Color::FromRGB(240, 240, 240);
		gNodes->style.colors[ColorVar_NodeBackgroundHovered]  = Color::FromRGB(240, 240, 240);
		gNodes->style.colors[ColorVar_NodeBackgroundSelected] = Color::FromRGB(240, 240, 240);
		gNodes->style.colors[ColorVar_NodeOutline]            = Color::FromRGB(100, 100, 100);
		gNodes->style.colors[ColorVar_TitleBar]               = Color::FromRGB(248, 248, 248);
		gNodes->style.colors[ColorVar_TitleBarHovered]        = Color::FromRGB(209, 209, 209);
		gNodes->style.colors[ColorVar_TitleBarSelected]       = Color::FromRGB(209, 209, 209);
		// original imgui values: 66, 150, 250
		gNodes->style.colors[ColorVar_Link] = Color::FromRGB(66, 150, 250, 100);
		// original imgui values: 117, 138, 204
		gNodes->style.colors[ColorVar_LinkHovered]  = Color::FromRGB(66, 150, 250, 242);
		gNodes->style.colors[ColorVar_LinkSelected] = Color::FromRGB(66, 150, 250, 242);
		// original imgui values: 66, 150, 250
		gNodes->style.colors[ColorVar_Pin]                = Color::FromRGB(66, 150, 250, 160);
		gNodes->style.colors[ColorVar_PinHovered]         = Color::FromRGB(66, 150, 250);
		gNodes->style.colors[ColorVar_BoxSelector]        = Color::FromRGB(90, 170, 250, 30);
		gNodes->style.colors[ColorVar_BoxSelectorOutline] = Color::FromRGB(90, 170, 250, 150);
		gNodes->style.colors[ColorVar_GridBackground]     = Color::FromRGB(225, 225, 225);
		gNodes->style.colors[ColorVar_GridLine]           = Color::FromRGB(180, 180, 180, 100);
		gNodes->style.colors[ColorVar_GridLinePrimary]    = Color::FromRGB(120, 120, 120, 100);

		// minimap colors
		gNodes->style.colors[ColorVar_MiniMapBackground]        = Color::FromRGB(25, 25, 25, 100);
		gNodes->style.colors[ColorVar_MiniMapBackgroundHovered] = Color::FromRGB(25, 25, 25, 200);
		gNodes->style.colors[ColorVar_MiniMapOutline]        = Color::FromRGB(150, 150, 150, 100);
		gNodes->style.colors[ColorVar_MiniMapOutlineHovered] = Color::FromRGB(150, 150, 150, 200);
		gNodes->style.colors[ColorVar_MiniMapNodeBackground] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    gNodes->style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    Color::FromRGB(200, 200, 240);
		gNodes->style.colors[ColorVar_MiniMapNodeOutline] = Color::FromRGB(200, 200, 200, 100);
		gNodes->style.colors[ColorVar_MiniMapLink]        = gNodes->style.colors[ColorVar_Link];
		gNodes->style.colors[ColorVar_MiniMapLinkSelected] =
		    gNodes->style.colors[ColorVar_LinkSelected];
		gNodes->style.colors[ColorVar_MiniMapCanvas]        = Color::FromRGB(200, 200, 200, 25);
		gNodes->style.colors[ColorVar_MiniMapCanvasOutline] = Color::FromRGB(200, 200, 200, 200);
	}

	void BeginNodeEditor()
	{
		assert(gNodes->currentScope == Scope::None);
		gNodes->currentScope = Scope::Editor;

		// Reset state from previous pass

		EditorContext& editor    = GetEditorContext();
		editor.AutoPanningDelta  = v2(0, 0);
		editor.gridContentBounds = Rect(v2{FLT_MAX, FLT_MAX}, v2{FLT_MIN, FLT_MIN});
		editor.miniMap.enabled   = false;

		editor.nodes.SwapFrameIds();
		ObjectPoolReset(editor.inputs);
		ObjectPoolReset(editor.outputs);
		ObjectPoolReset(editor.links);

		gNodes->hoveredNodeId = AST::NoId;
		gNodes->HoveredLinkIdx.Reset();
		gNodes->HoveredPinIdx = PinIdx::Invalid();
		gNodes->DeletedLinkIdx.Reset();
		gNodes->SnapLinkIdx.Reset();

		gNodes->nodeIdsOverlappingWithMouse.clear();

		gNodes->UIState = UIState_None;

		gNodes->mousePosition     = ImGui::GetIO().MousePos;
		gNodes->leftMouseClicked  = ImGui::IsMouseClicked(0);
		gNodes->leftMouseReleased = ImGui::IsMouseReleased(0);
		gNodes->leftMouseDragging = ImGui::IsMouseDragging(0, 0.0f);
		gNodes->altMouseClicked =
		    (gNodes->io.emulateThreeButtonMouse.modifier != nullptr
		        && *gNodes->io.emulateThreeButtonMouse.modifier && gNodes->leftMouseClicked)
		    || ImGui::IsMouseClicked(gNodes->io.AltMouseButton);
		gNodes->altMouseDragging =
		    (gNodes->io.emulateThreeButtonMouse.modifier != nullptr && gNodes->leftMouseDragging
		        && (*gNodes->io.emulateThreeButtonMouse.modifier))
		    || ImGui::IsMouseDragging(gNodes->io.AltMouseButton, 0.0f);
		gNodes->altMouseScrollDelta    = ImGui::GetIO().MouseWheel;
		gNodes->multipleSelectModifier = (gNodes->io.multipleSelectModifier.modifier != nullptr
		                                      ? *gNodes->io.multipleSelectModifier.modifier
		                                      : ImGui::GetIO().KeyCtrl);
		gNodes->activePin              = false;

		ImGui::BeginGroup();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, v2(1.f, 1.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, v2(0.f, 0.f));
			ImGui::PushStyleColor(
			    ImGuiCol_ChildBg, gNodes->style.colors[ColorVar_GridBackground].ToPackedABGR());
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

				if (gNodes->style.Flags & StyleFlags_GridLines)
				{
					DrawGrid(editor, canvasSize);
				}
			}
		}
	}

	void EndNodeEditor()
	{
		assert(gNodes->currentScope == Scope::Editor);
		gNodes->currentScope = Scope::None;

		EditorContext& editor = GetEditorContext();

		bool noGridContent = editor.gridContentBounds.IsInverted();
		if (noGridContent)
		{
			editor.gridContentBounds =
			    Rect{ScreenToGridPosition(editor, gNodes->CanvasRectScreenSpace.min),
			        ScreenToGridPosition(editor, gNodes->CanvasRectScreenSpace.max)};
		}

		// Detect ImGui i32eraction first, because it blocks i32eraction with the rest of the UI

		if (gNodes->leftMouseClicked && ImGui::IsAnyItemActive())
		{
			editor.clickInteraction.type = ClickInteractionType_ImGuiItem;
		}

		// Detect which UI element is being hovered over. Detection is done in a hierarchical
		// fashion, because a UI element being hovered excludes any other as being hovered over.

		// Don't do hovering detection for nodes/links/pins when i32eracting with the mini-map,
		// since its an *overlay* with its own i32eraction behavior and must have precedence
		// during mouse i32eraction.

		if ((editor.clickInteraction.type == ClickInteractionType_None
		        || editor.clickInteraction.type == ClickInteractionType_LinkCreation)
		    && IsMouseInCanvas() && !editor.miniMap.IsHovered())
		{
			// Pins needs some special care. We need to check the depth stack to see which pins
			// are being occluded by other nodes.
			ResolveOccludedPins(editor, gNodes->occludedPinIndices);

			gNodes->HoveredPinIdx =
			    ResolveHoveredPin(editor.outputs, PinType::Output, gNodes->occludedPinIndices);
			if (!gNodes->HoveredPinIdx)
			{
				gNodes->HoveredPinIdx =
				    ResolveHoveredPin(editor.inputs, PinType::Input, gNodes->occludedPinIndices);
			}

			if (!gNodes->HoveredPinIdx)
			{
				// Resolve which node is actually on top and being hovered using the depth
				// stack.
				gNodes->hoveredNodeId = ResolveHoveredNode(editor.nodes.depthOrder);
			}

			// We don't check for hovered pins here, because if we want to detach a link by
			// clicking and dragging, we need to have both a link and pin hovered.
			if (IsNone(gNodes->hoveredNodeId))
			{
				gNodes->HoveredLinkIdx =
				    ResolveHoveredLink(editor.links, editor.outputs, editor.inputs);
			}
		}

		for (AST::Id nodeId : editor.nodes)
		{
			DrawListActivateNodeBackground(nodeId);
			DrawNode(editor, nodeId);
		}

		// In order to render the links underneath the nodes, we want to first select the bottom
		// draw channel.
		gNodes->CanvasDrawList->ChannelsSetCurrent(0);

		for (i32 linkIdx = 0; linkIdx < editor.links.pool.Size(); ++linkIdx)
		{
			if (editor.links.inUse.IsSet(linkIdx))
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
			if (gNodes->leftMouseClicked && gNodes->HoveredLinkIdx.IsValid())
			{
				BeginLinkInteraction(editor, gNodes->HoveredLinkIdx.Value(), gNodes->HoveredPinIdx);
			}
			else if (gNodes->leftMouseClicked && gNodes->HoveredPinIdx)
			{
				BeginLinkCreation(editor, gNodes->HoveredPinIdx);
			}

			else if (gNodes->leftMouseClicked && !IsNone(gNodes->hoveredNodeId))
			{
				BeginNodeSelection(editor, gNodes->hoveredNodeId);
			}

			else if (gNodes->leftMouseClicked || gNodes->leftMouseReleased
			         || gNodes->altMouseClicked || gNodes->altMouseScrollDelta != 0.f)
			{
				BeginCanvasInteraction(editor);
			}

			bool shouldAutoPan = editor.clickInteraction.type == ClickInteractionType_BoxSelection
			                  || editor.clickInteraction.type == ClickInteractionType_LinkCreation
			                  || editor.clickInteraction.type == ClickInteractionType_Node;
			if (shouldAutoPan && !IsMouseInCanvas())
			{
				v2 mouse     = ImGui::GetMousePos();
				v2 center    = gNodes->CanvasRectScreenSpace.GetCenter();
				v2 direction = (center - mouse);
				direction    = direction * ImInvLength(direction, 0.0);

				editor.AutoPanningDelta =
				    direction * ImGui::GetIO().DeltaTime * gNodes->io.AutoPanningSpeed;
				editor.panning += editor.AutoPanningDelta;
			}
		}
		UpdateClickInteraction(editor);

		// At this point, draw commands have been issued for all nodes (and pins). Update the
		// node pool to detect unused node slots and remove those indices from the depth stack
		// before sorting the node draw commands by depth.
		for (AST::Id nodeId : editor.nodes)
		{
			auto& node = editor.nodes[nodeId];
			node.inputs.clear();
			node.outputs.clear();
		}
		editor.nodes.CacheInvalidIds();
		editor.nodes.ClearDepthOrder();

		ObjectPoolUpdate(editor.inputs);
		ObjectPoolUpdate(editor.outputs);

		DrawListSortChannelsByDepth(editor.nodes.depthOrder);

		// After the links have been rendered, the link pool can be updated as well.
		ObjectPoolUpdate(editor.links);

		// Finally, merge the draw channels
		gNodes->CanvasDrawList->ChannelsMerge();

		// pop style
		ImGui::EndChild();         // end scrolling region
		ImGui::PopStyleColor();    // pop child window background color
		ImGui::PopStyleVar();      // pop window padding
		ImGui::PopStyleVar();      // pop frame padding
		ImGui::EndGroup();
	}

	void BeginNode(const AST::Id nodeId)
	{
		// Remember to call BeginNodeEditor before calling BeginNode
		assert(gNodes->currentScope == Scope::Editor);
		gNodes->currentScope = Scope::Node;

		EditorContext& editor = GetEditorContext();

		gNodes->currentNodeId = nodeId;

		NodeData& node = editor.nodes.GetOrAdd(nodeId);

		node.colorStyle.Background         = gNodes->style.colors[ColorVar_NodeBackground];
		node.colorStyle.BackgroundHovered  = gNodes->style.colors[ColorVar_NodeBackgroundHovered];
		node.colorStyle.BackgroundSelected = gNodes->style.colors[ColorVar_NodeBackgroundSelected];
		node.colorStyle.Outline            = gNodes->style.colors[ColorVar_NodeOutline];
		node.colorStyle.Titlebar           = gNodes->style.colors[ColorVar_TitleBar];
		node.colorStyle.TitlebarHovered    = gNodes->style.colors[ColorVar_TitleBarHovered];
		node.colorStyle.TitlebarSelected   = gNodes->style.colors[ColorVar_TitleBarSelected];
		node.LayoutStyle.CornerRounding    = gNodes->style.NodeCornerRounding;
		node.LayoutStyle.Padding           = gNodes->style.NodePadding;
		node.LayoutStyle.BorderThickness   = gNodes->style.NodeBorderThickness;

		// ImGui::SetCursorPos sets the cursor position, local to the current widget
		// (in this case, the child object started in BeginNodeEditor). Use
		// ImGui::SetCursorScreenPos to set the screen space coordinates directly.
		ImGui::SetCursorPos(GridToEditorPosition(editor, GetNodeTitleBarOrigin(node)));

		DrawListAddNode(nodeId);
		DrawListActivateCurrentNodeForeground();

		ImGui::PushID(GetIdIndex(nodeId));
		ImGui::BeginGroup();
	}

	void EndNode()
	{
		assert(gNodes->currentScope == Scope::Node);
		gNodes->currentScope = Scope::Editor;

		EditorContext& editor = GetEditorContext();

		// The node's rectangle depends on the ImGui UI group size.
		ImGui::EndGroup();
		ImGui::PopID();

		NodeData& node = editor.nodes[gNodes->currentNodeId];
		node.rect      = GetItemRect();
		node.rect.Expand(node.LayoutStyle.Padding);

		editor.gridContentBounds.Merge(node.Origin);
		editor.gridContentBounds.Merge(node.Origin + node.rect.GetSize());

		if (node.rect.Contains(gNodes->mousePosition))
		{
			gNodes->nodeIdsOverlappingWithMouse.push_back(gNodes->currentNodeId);
		}
	}

	v2 GetNodeDimensions(AST::Id nodeId)
	{
		assert(!IsNone(nodeId));
		EditorContext& editor = GetEditorContext();
		const NodeData& node  = editor.nodes[nodeId];
		return node.rect.GetSize();
	}

	void BeginNodeTitleBar()
	{
		assert(gNodes->currentScope == Scope::Node);
		ImGui::BeginGroup();
	}

	void EndNodeTitleBar()
	{
		assert(gNodes->currentScope == Scope::Node);
		ImGui::EndGroup();

		EditorContext& editor    = GetEditorContext();
		NodeData& node           = editor.nodes[gNodes->currentNodeId];
		node.TitleBarContentRect = GetItemRect();

		Rect nodeTitleRect = GetNodeTitleRect(node);
		ImGui::ItemAdd({nodeTitleRect.min, nodeTitleRect.max}, ImGui::GetID("title_bar"));

		ImGui::SetCursorPos(GridToEditorPosition(editor, GetNodeContentOrigin(node)));
	}

	void BeginInput(const i32 id, const PinShape shape)
	{
		BeginPin(id, PinType::Input, shape, gNodes->currentNodeId);
	}

	void EndInput()
	{
		EndPin();
	}

	void BeginOutput(const i32 id, const PinShape shape)
	{
		BeginPin(id, PinType::Output, shape, gNodes->currentNodeId);
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

	void Link(i32 id, i32 outputPin, i32 inputPin)
	{
		assert(gNodes->currentScope == Scope::Editor);

		EditorContext& editor    = GetEditorContext();
		LinkData& link           = ObjectPoolFindOrCreateObject(editor.links, id);
		link.id                  = id;
		link.outputIdx           = ObjectPoolFindOrCreateIndex(editor.outputs, outputPin);
		link.inputIdx            = ObjectPoolFindOrCreateIndex(editor.inputs, inputPin);
		link.colorStyle.Base     = gNodes->style.colors[ColorVar_Link];
		link.colorStyle.Hovered  = gNodes->style.colors[ColorVar_LinkHovered];
		link.colorStyle.Selected = gNodes->style.colors[ColorVar_LinkSelected];

		// Check if this link was created by the current link event
		if ((editor.clickInteraction.type == ClickInteractionType_LinkCreation
		        && editor.inputs.pool[link.inputIdx].Flags & PinFlags_EnableLinkCreationOnSnap
		        && editor.clickInteraction.linkCreation.outputIdx == link.outputIdx
		        && editor.clickInteraction.linkCreation.inputIdx == link.inputIdx)
		    || (editor.clickInteraction.linkCreation.outputIdx == link.inputIdx
		        && editor.clickInteraction.linkCreation.inputIdx == link.outputIdx))
		{
			gNodes->SnapLinkIdx = ObjectPoolFindOrCreateIndex(editor.links, id);
		}
	}

	void PushStyleColor(const ColorVar item, Color color)
	{
		gNodes->ColorModifierStack.push_back(ColElement(item, gNodes->style.colors[item]));
		gNodes->style.colors[item] = color;
	}

	void PopStyleColor(i32 count)
	{
		assert(gNodes->ColorModifierStack.size() >= count);
		while (count > 0)
		{
			const ColElement& elem          = gNodes->ColorModifierStack.back();
			gNodes->style.colors[elem.item] = elem.color;
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
 // StyleVar_PinDiamondSideLength
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, PinDiamondSideLength)     },
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
			float& styleVar = *(float*)varInfo->GetVarPtr(&gNodes->style);
			gNodes->styleModifierStack.push_back(StyleVarElement(item, styleVar));
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
			v2& styleVar = *(v2*)varInfo->GetVarPtr(&gNodes->style);
			gNodes->styleModifierStack.push_back(StyleVarElement(item, styleVar));
			styleVar = value;
			return;
		}
		IM_ASSERT(0 && "Called PushStyleVar() v2 variant but variable is not a v2!");
	}

	void PopStyleVar(i32 count)
	{
		while (count > 0)
		{
			assert(gNodes->styleModifierStack.size() > 0);
			const StyleVarElement styleBackup = gNodes->styleModifierStack.back();
			gNodes->styleModifierStack.pop_back();
			const StyleVarInfo* varInfo = GetStyleVarInfo(styleBackup.item);
			void* styleVar              = varInfo->GetVarPtr(&gNodes->style);
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

	void SetNodeScreenSpacePos(AST::Id nodeId, const v2& screenSpacePos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = editor.nodes.GetOrAdd(nodeId);
		node.Origin           = ScreenToGridPosition(editor, screenSpacePos);
	}

	void SetNodeEditorSpacePos(AST::Id nodeId, const v2& editorSpacePos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = editor.nodes.GetOrAdd(nodeId);
		node.Origin           = EditorToGridPosition(editor, editorSpacePos);
	}

	void SetNodeGridSpacePos(AST::Id nodeId, const v2& gridPos)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = editor.nodes.GetOrAdd(nodeId);
		node.Origin           = gridPos;
	}

	void SetNodeDraggable(AST::Id nodeId, const bool draggable)
	{
		EditorContext& editor = GetEditorContext();
		NodeData& node        = editor.nodes.GetOrAdd(nodeId);
		node.Draggable        = draggable;
	}

	v2 GetNodeScreenSpacePos(AST::Id nodeId)
	{
		assert(!IsNone(nodeId));
		EditorContext& editor = GetEditorContext();
		const NodeData& node  = editor.nodes[nodeId];
		return GridToScreenPosition(editor, node.Origin);
	}

	v2 GetNodeEditorSpacePos(AST::Id nodeId)
	{
		assert(!IsNone(nodeId));
		EditorContext& editor = GetEditorContext();
		const NodeData& node  = editor.nodes[nodeId];
		return GridToEditorPosition(editor, node.Origin);
	}

	v2 GetNodeGridSpacePos(AST::Id nodeId)
	{
		assert(!IsNone(nodeId));
		EditorContext& editor = GetEditorContext();
		const NodeData& node  = editor.nodes[nodeId];
		return node.Origin;
	}

	bool IsEditorHovered()
	{
		return IsMouseInCanvas();
	}

	AST::Id GetHoveredNode()
	{
		return gNodes->hoveredNodeId;
	}

	AST::Id GetHoveredLink()
	{
		if (gNodes->HoveredLinkIdx.IsValid())
		{
			const EditorContext& editor = GetEditorContext();
			return AST::Id(editor.links.pool[gNodes->HoveredLinkIdx.Value()].id);
		}
		return AST::NoId;
	}

	bool IsNodeHovered(AST::Id nodeId)
	{
		assert(gNodes->currentScope != Scope::None);
		return gNodes->hoveredNodeId == nodeId && gNodes->hoveredNodeId != AST::NoId;
	}

	bool IsLinkHovered(AST::Id linkId)
	{
		assert(gNodes->currentScope != Scope::None);

		const EditorContext& editor = GetEditorContext();
		return gNodes->HoveredLinkIdx.IsValid()
		    && linkId == AST::Id(editor.links.pool[gNodes->HoveredLinkIdx.Value()].id);
	}

	bool IsPinHovered(Id* const pin)
	{
		assert(gNodes->currentScope != Scope::None);
		assert(pin != nullptr);

		const bool isHovered = gNodes->HoveredPinIdx;
		if (gNodes->HoveredPinIdx)
		{
			const EditorContext& editor = GetEditorContext();
			*pin                        = editor.GetPinData(gNodes->HoveredPinIdx).id;
			return true;
		}
		return false;
	}

	i32 NumSelectedNodes()
	{
		assert(gNodes->currentScope != Scope::None);
		const EditorContext& editor = GetEditorContext();
		return editor.selectedNodeIds.Size();
	}

	i32 NumSelectedLinks()
	{
		assert(gNodes->currentScope != Scope::None);
		const EditorContext& editor = GetEditorContext();
		return editor.selectedLinkIndices.size();
	}

	const TArray<AST::Id>& GetSelectedNodes()
	{
		const EditorContext& editor = GetEditorContext();
		return editor.selectedNodeIds;
	}

	bool GetSelectedLinks(TArray<AST::Id>& linkIds)
	{
		const EditorContext& editor = GetEditorContext();
		linkIds.Resize(editor.selectedLinkIndices.size());
		for (i32 i = 0; i < editor.selectedLinkIndices.size(); ++i)
		{
			const i32 linkIdx = editor.selectedLinkIndices[i];
			linkIds[i]        = AST::Id(editor.links.pool[linkIdx].id);
		}
		return !linkIds.IsEmpty();
	}

	void ClearNodeSelection()
	{
		EditorContext& editor = GetEditorContext();
		editor.selectedNodeIds.Clear();
	}

	void ClearNodeSelection(AST::Id nodeId)
	{
		EditorContext& editor = GetEditorContext();
		editor.selectedNodeIds.Remove(nodeId);
	}

	void ClearLinkSelection()
	{
		EditorContext& editor = GetEditorContext();
		editor.selectedLinkIndices.clear();
	}

	void ClearLinkSelection(Id linkId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 idx         = ObjectPoolFind(editor.links, linkId);
		assert(idx >= 0);
		assert(editor.selectedLinkIndices.find(idx) != editor.selectedLinkIndices.end());
		editor.selectedLinkIndices.find_erase_unsorted(idx);
	}

	void SelectNode(AST::Id nodeId)
	{
		EditorContext& editor = GetEditorContext();
		editor.selectedNodeIds.AddUnique(nodeId);
	}

	void SelectLink(Id linkId)
	{
		EditorContext& editor = GetEditorContext();
		const i32 idx         = ObjectPoolFind(editor.links, linkId);
		assert(idx >= 0);
		assert(editor.selectedLinkIndices.find(idx) == editor.selectedLinkIndices.end());
		editor.selectedLinkIndices.push_back(idx);
	}

	bool IsNodeSelected(AST::Id nodeId)
	{
		EditorContext& editor = GetEditorContext();
		return editor.selectedNodeIds.Contains(nodeId);
	}

	bool IsLinkSelected(Id linkId)
	{
		EditorContext& editor = GetEditorContext();
		return IsObjectSelected(editor.links, editor.selectedLinkIndices, linkId);
	}

	bool IsLinkSelectedByIdx(i32 linkIdx)
	{
		EditorContext& editor = GetEditorContext();
		return IsObjectSelectedByIdx(editor.links, editor.selectedLinkIndices, linkIdx);
	}

	bool IsPinActive()
	{
		Check(HasFlag(gNodes->currentScope, Scope::Node));

		if (!gNodes->activePin)
		{
			return false;
		}

		return gNodes->activePinId == gNodes->CurrentPinId;
	}

	bool IsAnyPinActive(Id* const pinId)
	{
		Check(!HasAnyFlags(gNodes->currentScope, (Scope::Node | Scope::Pin)));

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

	bool IsDraggingLink()
	{
		return (gNodes->UIState & UIState_LinkStarted) != 0;
	}
	TPair<Id, PinType> GetDraggedOriginPin()
	{
		if (IsDraggingLink())
		{
			const EditorContext& editor = GetEditorContext();

			const i32 outputIdx = editor.clickInteraction.linkCreation.outputIdx;
			if (outputIdx != NO_INDEX)
			{
				return {editor.GetPinData({outputIdx, PinType::Output}).id, PinType::Output};
			}
			const i32 inputIdx = editor.clickInteraction.linkCreation.inputIdx;
			if (inputIdx != NO_INDEX)
			{
				return {editor.GetPinData({inputIdx, PinType::Input}).id, PinType::Input};
			}
		}
		return {0, PinType::None};
	}

	bool IsLinkDropped(Id* outputId, Id* inputId, bool includingDetachedLinks)
	{
		// Call this function after EndNodeEditor()!
		Check(gNodes->currentScope != Scope::None);
		Check(outputId != nullptr);

		const EditorContext& editor = GetEditorContext();

		const bool linkDropped =
		    (gNodes->UIState & UIState_LinkDropped) != 0
		    && (includingDetachedLinks
		        || editor.clickInteraction.linkCreation.type != LinkCreationType_FromDetach);

		if (linkDropped)
		{
			const i32 outputIdx = editor.clickInteraction.linkCreation.outputIdx;
			if (outputId && outputIdx != NO_INDEX)
			{
				*outputId = editor.GetPinData({outputIdx, PinType::Output}).id;
			}
			const i32 inputIdx = editor.clickInteraction.linkCreation.inputIdx;
			if (inputId && inputIdx != NO_INDEX)
			{
				*inputId = editor.GetPinData({inputIdx, PinType::Input}).id;
			}
		}

		return linkDropped;
	}

	bool IsLinkCreated(Id& outputPinId, Id& inputPinId, bool* createdFromSnap)
	{
		Check(gNodes->currentScope == Scope::None);

		if ((gNodes->UIState & UIState_LinkCreated) != 0)
		{
			const EditorContext& editor = GetEditorContext();
			const PinData& outputData =
			    editor.GetPinData(PinIdx::Output(editor.clickInteraction.linkCreation.outputIdx));
			const PinData& inputData =
			    editor.GetPinData(PinIdx::Input(editor.clickInteraction.linkCreation.inputIdx));

			outputPinId = outputData.id;
			inputPinId  = inputData.id;

			if (createdFromSnap)
			{
				*createdFromSnap =
				    editor.clickInteraction.type == ClickInteractionType_LinkCreation;
			}
			return true;
		}
		return false;
	}

	bool IsLinkCreated(AST::Id& outputNodeId, Id& outputPinId, AST::Id& inputNodeId, Id& inputPinId,
	    bool* createdFromSnap)
	{
		Check(gNodes->currentScope == Scope::None);

		if ((gNodes->UIState & UIState_LinkCreated) != 0)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 outputIdx         = editor.clickInteraction.linkCreation.outputIdx;
			const i32 inputIdx          = editor.clickInteraction.linkCreation.inputIdx;
			const PinData& outputData   = editor.outputs.pool[outputIdx];
			const PinData& inputData    = editor.inputs.pool[inputIdx];

			outputPinId  = outputData.id;
			outputNodeId = outputData.parentNodeId;
			inputPinId   = inputData.id;
			inputNodeId  = inputData.parentNodeId;

			if (createdFromSnap)
			{
				*createdFromSnap =
				    editor.clickInteraction.type == ClickInteractionType_LinkCreation;
			}
			return true;
		}
		return false;
	}

	bool IsLinkDestroyed(Id& linkId)
	{
		Check(gNodes->currentScope == Scope::None);

		const bool linkDestroyed = gNodes->DeletedLinkIdx.IsValid();
		if (linkDestroyed)
		{
			const EditorContext& editor = GetEditorContext();
			const i32 linkIdx           = gNodes->DeletedLinkIdx.Value();
			linkId                      = editor.links.pool[linkIdx].id;
		}

		return linkDestroyed;
	}
}    // namespace rift::Nodes
