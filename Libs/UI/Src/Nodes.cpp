// the structure of this file:
//
// [SECTION] bezier curve helpers
// [SECTION] draw list helper
// [SECTION] ui state logic
// [SECTION] render helpers
// [SECTION] API implementation

#include "UI/Nodes.h"
#include "UI/NodesInternal.h"
#include "UI/UIImGui.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>    // strlen, strncmp

#include <new>


namespace Rift::Nodes
{
	Context* GNodes = nullptr;

	// [SECTION] bezier curve helpers

	struct CubicBezier
	{
		v2 p0, p1, p2, p3;
		i32 NumSegments;
	};

	v2 EvalCubicBezier(const float t, const v2& p0, const v2& p1, const v2& p2, const v2& p3)
	{
		// B(t) = (1-t)**3 p0 + 3(1 - t)**2 t p1 + 3(1-t)t**2 p2 + t**3 p3

		const float u  = 1.0f - t;
		const float b0 = u * u * u;
		const float b1 = 3 * u * u * t;
		const float b2 = 3 * u * t * t;
		const float b3 = t * t * t;
		return v2(b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
		    b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y);
	}

	// Calculates the closest point along each bezier curve segment.
	v2 GetClosestPointOnCubicBezier(const i32 num_segments, const v2& p, const CubicBezier& cb)
	{
		IM_ASSERT(num_segments > 0);
		v2 p_last = cb.p0;
		v2 p_closest;
		float p_closest_dist = FLT_MAX;
		float t_step         = 1.0f / (float)num_segments;
		for (i32 i = 1; i <= num_segments; ++i)
		{
			v2 p_current = EvalCubicBezier(t_step * i, cb.p0, cb.p1, cb.p2, cb.p3);
			v2 p_line    = Vectors::ClosestPointInLine(p_last, p_current, p);
			float dist   = (p - p_line).LengthSquared();
			if (dist < p_closest_dist)
			{
				p_closest      = p_line;
				p_closest_dist = dist;
			}
			p_last = p_current;
		}
		return p_closest;
	}

	float GetDistanceToCubicBezier(
	    const v2& pos, const CubicBezier& cubic_bezier, const i32 num_segments)
	{
		const v2 point_on_curve = GetClosestPointOnCubicBezier(num_segments, pos, cubic_bezier);

		const v2 to_curve = point_on_curve - pos;
		return ImSqrt(ImLengthSqr(to_curve));
	}

	Rect GetContainingRectForCubicBezier(const CubicBezier& cb)
	{
		const v2 min = v2(ImMin(cb.p0.x, cb.p3.x), ImMin(cb.p0.y, cb.p3.y));
		const v2 max = v2(ImMax(cb.p0.x, cb.p3.x), ImMax(cb.p0.y, cb.p3.y));

		const float hover_distance = GNodes->Style.LinkHoverDistance;

		Rect rect(min, max);
		rect.Add(cb.p1);
		rect.Add(cb.p2);
		rect.Expand(v2{hover_distance, hover_distance});

		return rect;
	}

	CubicBezier GetCubicBezier(
	    v2 start, v2 end, const AttributeType start_type, const float line_segments_per_length)
	{
		assert((start_type == AttributeType_Input) || (start_type == AttributeType_Output));
		if (start_type == AttributeType_Input)
		{
			ImSwap(start, end);
		}

		const float link_length = ImSqrt(ImLengthSqr(end - start));
		const v2 offset         = v2(0.25f * link_length, 0.f);
		CubicBezier cubic_bezier;
		cubic_bezier.p0 = start;
		cubic_bezier.p1 = start + offset;
		cubic_bezier.p2 = end - offset;
		cubic_bezier.p3 = end;
		cubic_bezier.NumSegments =
		    ImMax(static_cast<i32>(link_length * line_segments_per_length), 1);
		return cubic_bezier;
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
		Rect flip_rect = rect;

		if (flip_rect.min.x > flip_rect.max.x)
		{
			ImSwap(flip_rect.min.x, flip_rect.max.x);
		}

		if (flip_rect.min.y > flip_rect.max.y)
		{
			ImSwap(flip_rect.min.y, flip_rect.max.y);
		}

		// Trivial case: line segment lies to one particular side of rectangle
		if ((p1.x < flip_rect.min.x && p2.x < flip_rect.min.x)
		    || (p1.x > flip_rect.max.x && p2.x > flip_rect.max.x)
		    || (p1.y < flip_rect.min.y && p2.y < flip_rect.min.y)
		    || (p1.y > flip_rect.max.y && p2.y > flip_rect.max.y))
		{
			return false;
		}

		const i32 corner_signs[4] = {Sign(EvalImplicitLineEq(p1, p2, flip_rect.min)),
		    Sign(EvalImplicitLineEq(p1, p2, v2(flip_rect.max.x, flip_rect.min.y))),
		    Sign(EvalImplicitLineEq(p1, p2, v2(flip_rect.min.x, flip_rect.max.y))),
		    Sign(EvalImplicitLineEq(p1, p2, flip_rect.max))};

		i32 sum     = 0;
		i32 sum_abs = 0;

		for (i32 i = 0; i < 4; ++i)
		{
			sum += corner_signs[i];
			sum_abs += abs(corner_signs[i]);
		}

		// At least one corner of rectangle lies on a different side of line segment
		return abs(sum) != sum_abs;
	}

	bool RectangleOverlapsBezier(const Rect& rectangle, const CubicBezier& cubic_bezier)
	{
		v2 current = EvalCubicBezier(
		    0.f, cubic_bezier.p0, cubic_bezier.p1, cubic_bezier.p2, cubic_bezier.p3);
		const float dt = 1.0f / cubic_bezier.NumSegments;
		for (i32 s = 0; s < cubic_bezier.NumSegments; ++s)
		{
			v2 next = EvalCubicBezier(static_cast<float>((s + 1) * dt), cubic_bezier.p0,
			    cubic_bezier.p1, cubic_bezier.p2, cubic_bezier.p3);
			if (RectangleOverlapsLineSegment(rectangle, current, next))
			{
				return true;
			}
			current = next;
		}
		return false;
	}

	bool RectangleOverlapsLink(
	    const Rect& rectangle, const v2& start, const v2& end, const AttributeType start_type)
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

			const CubicBezier cubic_bezier =
			    GetCubicBezier(start, end, start_type, GNodes->Style.LinkLineSegmentsPerLength);
			return RectangleOverlapsBezier(rectangle, cubic_bezier);
		}

		return false;
	}

	// [SECTION] coordinate space conversion helpers

	v2 ScreenSpaceToGridSpace(const EditorContext& editor, const v2& v)
	{
		return v - GNodes->CanvasOriginScreenSpace - editor.Panning;
	}

	Rect ScreenSpaceToGridSpace(const EditorContext& editor, const Rect& r)
	{
		return Rect(ScreenSpaceToGridSpace(editor, r.min), ScreenSpaceToGridSpace(editor, r.max));
	}

	v2 GridSpaceToScreenSpace(const EditorContext& editor, const v2& v)
	{
		return v + GNodes->CanvasOriginScreenSpace + editor.Panning;
	}

	v2 GridSpaceToEditorSpace(const EditorContext& editor, const v2& v)
	{
		return v + editor.Panning;
	}

	v2 EditorSpaceToGridSpace(const EditorContext& editor, const v2& v)
	{
		return v - editor.Panning;
	}

	v2 EditorSpaceToScreenSpace(const v2& v)
	{
		return GNodes->CanvasOriginScreenSpace + v;
	}

	v2 MiniMapSpaceToGridSpace(const EditorContext& editor, const v2& v)
	{
		return (v - editor.miniMapContentScreenSpace.min) / editor.miniMapScaling
		       + editor.gridContentBounds.min;
	};

	v2 ScreenSpaceToMiniMapSpace(const EditorContext& editor, const v2& v)
	{
		return (ScreenSpaceToGridSpace(editor, v) - editor.gridContentBounds.min)
		           * editor.miniMapScaling
		       + editor.miniMapContentScreenSpace.min;
	};

	Rect ScreenSpaceToMiniMapSpace(const EditorContext& editor, const Rect& r)
	{
		return Rect(
		    ScreenSpaceToMiniMapSpace(editor, r.min), ScreenSpaceToMiniMapSpace(editor, r.max));
	};

	// [SECTION] draw list helper

	void ImDrawListGrowChannels(ImDrawList* draw_list, const i32 num_channels)
	{
		ImDrawListSplitter& splitter = draw_list->_Splitter;

		if (splitter._Count == 1)
		{
			splitter.Split(draw_list, num_channels + 1);
			return;
		}

		// NOTE: this logic has been lifted from ImDrawListSplitter::Split with slight
		// modifications to allow nested splits. The main modification is that we only create
		// new ImDrawChannel instances after splitter._Count, instead of over the whole
		// splitter._Channels array like the regular ImDrawListSplitter::Split method does.

		const i32 old_channel_capacity = splitter._Channels.Size;
		// NOTE: _Channels is not resized down, and therefore _Count <= _Channels.size()!
		const i32 old_channel_count       = splitter._Count;
		const i32 requested_channel_count = old_channel_count + num_channels;
		if (old_channel_capacity < old_channel_count + num_channels)
		{
			splitter._Channels.resize(requested_channel_count);
		}

		splitter._Count = requested_channel_count;

		for (i32 i = old_channel_count; i < requested_channel_count; ++i)
		{
			ImDrawChannel& channel = splitter._Channels[i];

			// If we're inside the old capacity region of the array, we need to reuse the
			// existing memory of the command and index buffers.
			if (i < old_channel_capacity)
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
				ImDrawCmd draw_cmd;
				draw_cmd.ClipRect  = draw_list->_ClipRectStack.back();
				draw_cmd.TextureId = draw_list->_TextureIdStack.back();
				channel._CmdBuffer.push_back(draw_cmd);
			}
		}
	}

	void ImDrawListSplitterSwapChannels(
	    ImDrawListSplitter& splitter, const i32 lhs_idx, const i32 rhs_idx)
	{
		if (lhs_idx == rhs_idx)
		{
			return;
		}

		assert(lhs_idx >= 0 && lhs_idx < splitter._Count);
		assert(rhs_idx >= 0 && rhs_idx < splitter._Count);

		ImDrawChannel& lhs_channel = splitter._Channels[lhs_idx];
		ImDrawChannel& rhs_channel = splitter._Channels[rhs_idx];
		lhs_channel._CmdBuffer.swap(rhs_channel._CmdBuffer);
		lhs_channel._IdxBuffer.swap(rhs_channel._IdxBuffer);

		const i32 current_channel = splitter._Current;

		if (current_channel == lhs_idx)
		{
			splitter._Current = rhs_idx;
		}
		else if (current_channel == rhs_idx)
		{
			splitter._Current = lhs_idx;
		}
	}

	void DrawListSet(ImDrawList* window_draw_list)
	{
		GNodes->CanvasDrawList = window_draw_list;
		GNodes->NodeIdxToSubmissionIdx.Clear();
		GNodes->NodeIdxSubmissionOrder.clear();
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

	void DrawListAddNode(const i32 node_idx)
	{
		GNodes->NodeIdxToSubmissionIdx.SetInt(
		    static_cast<ImGuiID>(node_idx), GNodes->NodeIdxSubmissionOrder.Size);
		GNodes->NodeIdxSubmissionOrder.push_back(node_idx);
		ImDrawListGrowChannels(GNodes->CanvasDrawList, 2);
	}

	void DrawListAppendClickInteractionChannel()
	{
		// NOTE: don't use this function outside of EndNodeEditor. Using this before all nodes
		// have been added will screw up the node draw order.
		ImDrawListGrowChannels(GNodes->CanvasDrawList, 1);
	}

	i32 DrawListSubmissionIdxToBackgroundChannelIdx(const i32 submission_idx)
	{
		// NOTE: the first channel is the canvas background, i.e. the grid
		return 1 + 2 * submission_idx;
	}

	i32 DrawListSubmissionIdxToForegroundChannelIdx(const i32 submission_idx)
	{
		return DrawListSubmissionIdxToBackgroundChannelIdx(submission_idx) + 1;
	}

	void DrawListActivateClickInteractionChannel()
	{
		GNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    GNodes->CanvasDrawList, GNodes->CanvasDrawList->_Splitter._Count - 1);
	}

	void DrawListActivateCurrentNodeForeground()
	{
		const i32 foreground_channel_idx =
		    DrawListSubmissionIdxToForegroundChannelIdx(GNodes->NodeIdxSubmissionOrder.Size - 1);
		GNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    GNodes->CanvasDrawList, foreground_channel_idx);
	}

	void DrawListActivateNodeBackground(const i32 node_idx)
	{
		const i32 submission_idx =
		    GNodes->NodeIdxToSubmissionIdx.GetInt(static_cast<ImGuiID>(node_idx), -1);
		// There is a discrepancy in the submitted node count and the rendered node count! Did
		// you call one of the following functions
		// * EditorContextMoveToNode
		// * SetNodeScreenSpacePos
		// * SetNodeGridSpacePos
		// * SetNodeDraggable
		// after the BeginNode/EndNode function calls?
		assert(submission_idx != -1);
		const i32 background_channel_idx =
		    DrawListSubmissionIdxToBackgroundChannelIdx(submission_idx);
		GNodes->CanvasDrawList->_Splitter.SetCurrentChannel(
		    GNodes->CanvasDrawList, background_channel_idx);
	}

	void DrawListSwapSubmissionIndices(const i32 lhs_idx, const i32 rhs_idx)
	{
		assert(lhs_idx != rhs_idx);

		const i32 lhs_foreground_channel_idx = DrawListSubmissionIdxToForegroundChannelIdx(lhs_idx);
		const i32 lhs_background_channel_idx = DrawListSubmissionIdxToBackgroundChannelIdx(lhs_idx);
		const i32 rhs_foreground_channel_idx = DrawListSubmissionIdxToForegroundChannelIdx(rhs_idx);
		const i32 rhs_background_channel_idx = DrawListSubmissionIdxToBackgroundChannelIdx(rhs_idx);

		ImDrawListSplitterSwapChannels(GNodes->CanvasDrawList->_Splitter,
		    lhs_background_channel_idx, rhs_background_channel_idx);
		ImDrawListSplitterSwapChannels(GNodes->CanvasDrawList->_Splitter,
		    lhs_foreground_channel_idx, rhs_foreground_channel_idx);
	}

	void DrawListSortChannelsByDepth(const ImVector<i32>& node_idx_depth_order)
	{
		if (GNodes->NodeIdxToSubmissionIdx.Data.Size < 2)
		{
			return;
		}

		assert(node_idx_depth_order.Size == GNodes->NodeIdxSubmissionOrder.Size);

		i32 start_idx = node_idx_depth_order.Size - 1;

		while (node_idx_depth_order[start_idx] == GNodes->NodeIdxSubmissionOrder[start_idx])
		{
			if (--start_idx == 0)
			{
				// early out if submission order and depth order are the same
				return;
			}
		}

		// TODO: this is an O(N^2) algorithm. It might be worthwhile revisiting this to see if
		// the time complexity can be reduced.

		for (i32 depth_idx = start_idx; depth_idx > 0; --depth_idx)
		{
			const i32 node_idx = node_idx_depth_order[depth_idx];

			// Find the current index of the node_idx in the submission order array
			i32 submission_idx = -1;
			for (i32 i = 0; i < GNodes->NodeIdxSubmissionOrder.Size; ++i)
			{
				if (GNodes->NodeIdxSubmissionOrder[i] == node_idx)
				{
					submission_idx = i;
					break;
				}
			}
			assert(submission_idx >= 0);

			if (submission_idx == depth_idx)
			{
				continue;
			}

			for (i32 j = submission_idx; j < depth_idx; ++j)
			{
				DrawListSwapSubmissionIndices(j, j + 1);
				ImSwap(GNodes->NodeIdxSubmissionOrder[j], GNodes->NodeIdxSubmissionOrder[j + 1]);
			}
		}
	}

	// [SECTION] ui state logic

	v2 GetScreenSpacePinCoordinates(
	    const Rect& node_rect, const Rect& attribute_rect, const AttributeType type)
	{
		assert(type == AttributeType_Input || type == AttributeType_Output);
		const float x = type == AttributeType_Input ? (node_rect.min.x - GNodes->Style.PinOffset)
		                                            : (node_rect.max.x + GNodes->Style.PinOffset);
		return v2(x, 0.5f * (attribute_rect.min.y + attribute_rect.max.y));
	}

	v2 GetScreenSpacePinCoordinates(const EditorContext& editor, const PinData& pin)
	{
		const Rect& parent_node_rect = editor.Nodes.Pool[pin.ParentNodeIdx].Rect;
		return GetScreenSpacePinCoordinates(parent_node_rect, pin.AttributeRect, pin.Type);
	}

	bool MouseInCanvas()
	{
		// This flag should be true either when hovering or clicking something in the canvas.
		const bool is_window_hovered_or_focused =
		    ImGui::IsWindowHovered() || ImGui::IsWindowFocused();

		return is_window_hovered_or_focused
		       && GNodes->CanvasRectScreenSpace.Contains(ImGui::GetMousePos());
	}

	void BeginNodeSelection(EditorContext& editor, const i32 node_idx)
	{
		// Don't start selecting a node if we are e.g. already creating and dragging
		// a new link! New link creation can happen when the mouse is clicked over
		// a node, but within the hover radius of a pin.
		if (editor.ClickInteraction.Type != ClickInteractionType_None)
		{
			return;
		}

		editor.ClickInteraction.Type = ClickInteractionType_Node;
		// If the node is not already contained in the selection, then we want only
		// the i32eraction node to be selected, effective immediately.
		//
		// If the multiple selection modifier is active, we want to add this node
		// to the current list of selected nodes.
		//
		// Otherwise, we want to allow for the possibility of multiple nodes to be
		// moved at once.
		if (!editor.SelectedNodeIndices.contains(node_idx))
		{
			editor.SelectedLinkIndices.clear();
			if (!GNodes->MultipleSelectModifier)
				editor.SelectedNodeIndices.clear();
			editor.SelectedNodeIndices.push_back(node_idx);

			// Ensure that individually selected nodes get rendered on top
			ImVector<i32>& depth_stack = editor.NodeDepthOrder;
			const i32* const elem      = depth_stack.find(node_idx);
			assert(elem != depth_stack.end());
			depth_stack.erase(elem);
			depth_stack.push_back(node_idx);
		}
		// Deselect a previously-selected node
		else if (GNodes->MultipleSelectModifier)
		{
			const i32* const node_ptr = editor.SelectedNodeIndices.find(node_idx);
			editor.SelectedNodeIndices.erase(node_ptr);

			// Don't allow dragging after deselecting
			editor.ClickInteraction.Type = ClickInteractionType_None;
		}

		// To support snapping of multiple nodes, we need to store the offset of
		// each node in the selection to the origin of the dragged node.
		const v2 ref_origin = editor.Nodes.Pool[node_idx].Origin;
		editor.PrimaryNodeOffset =
		    ref_origin + GNodes->CanvasOriginScreenSpace + editor.Panning - GNodes->MousePos;

		editor.SelectedNodeOrigins.clear();
		for (i32 idx = 0; idx < editor.SelectedNodeIndices.Size; idx++)
		{
			const i32 node       = editor.SelectedNodeIndices[idx];
			const v2 node_origin = editor.Nodes.Pool[node].Origin - ref_origin;
			editor.SelectedNodeOrigins.push_back(node_origin);
		}
	}

	void BeginLinkSelection(EditorContext& editor, const i32 link_idx)
	{
		editor.ClickInteraction.Type = ClickInteractionType_Link;
		// When a link is selected, clear all other selections, and insert the link
		// as the sole selection.
		editor.SelectedNodeIndices.clear();
		editor.SelectedLinkIndices.clear();
		editor.SelectedLinkIndices.push_back(link_idx);
	}

	void BeginLinkDetach(EditorContext& editor, const i32 link_idx, const i32 detach_pin_idx)
	{
		const ImLinkData& link         = editor.Links.Pool[link_idx];
		ImClickInteractionState& state = editor.ClickInteraction;
		state.Type                     = ClickInteractionType_LinkCreation;
		state.LinkCreation.EndPinIdx.Reset();
		state.LinkCreation.StartPinIdx =
		    detach_pin_idx == link.StartPinIdx ? link.EndPinIdx : link.StartPinIdx;
		GNodes->DeletedLinkIdx = link_idx;
	}

	void BeginLinkCreation(EditorContext& editor, const i32 hovered_pin_idx)
	{
		editor.ClickInteraction.Type                     = ClickInteractionType_LinkCreation;
		editor.ClickInteraction.LinkCreation.StartPinIdx = hovered_pin_idx;
		editor.ClickInteraction.LinkCreation.EndPinIdx.Reset();
		editor.ClickInteraction.LinkCreation.Type = LinkCreationType_Standard;
		GNodes->UIState |= UIState_LinkStarted;
	}

	void BeginLinki32eraction(EditorContext& editor, const i32 link_idx,
	    const ImOptionalIndex pin_idx = ImOptionalIndex())
	{
		// Check if we are clicking the link with the modifier pressed.
		// This will in a link detach via clicking.

		const bool modifier_pressed = GNodes->Io.LinkDetachWithModifierClick.Modifier == nullptr
		                                  ? false
		                                  : *GNodes->Io.LinkDetachWithModifierClick.Modifier;

		if (modifier_pressed)
		{
			const ImLinkData& link    = editor.Links.Pool[link_idx];
			const PinData& start_pin  = editor.Pins.Pool[link.StartPinIdx];
			const PinData& end_pin    = editor.Pins.Pool[link.EndPinIdx];
			const v2& mouse_pos       = GNodes->MousePos;
			const float dist_to_start = ImLengthSqr(start_pin.Pos - mouse_pos);
			const float dist_to_end   = ImLengthSqr(end_pin.Pos - mouse_pos);
			const i32 closest_pin_idx =
			    dist_to_start < dist_to_end ? link.StartPinIdx : link.EndPinIdx;

			editor.ClickInteraction.Type = ClickInteractionType_LinkCreation;
			BeginLinkDetach(editor, link_idx, closest_pin_idx);
			editor.ClickInteraction.LinkCreation.Type = LinkCreationType_FromDetach;
		}
		else
		{
			if (pin_idx.HasValue())
			{
				const i32 hovered_pin_flags = editor.Pins.Pool[pin_idx.Value()].Flags;

				// Check the 'click and drag to detach' case.
				if (hovered_pin_flags & AttributeFlags_EnableLinkDetachWithDragClick)
				{
					BeginLinkDetach(editor, link_idx, pin_idx.Value());
					editor.ClickInteraction.LinkCreation.Type = LinkCreationType_FromDetach;
				}
				else
				{
					BeginLinkCreation(editor, pin_idx.Value());
				}
			}
			else
			{
				BeginLinkSelection(editor, link_idx);
			}
		}
	}

	static bool IsMiniMapHovered();

	void BeginCanvasi32eraction(EditorContext& editor)
	{
		const bool any_ui_element_hovered =
		    GNodes->HoveredNodeIdx.HasValue() || GNodes->HoveredLinkIdx.HasValue()
		    || GNodes->HoveredPinIdx.HasValue() || ImGui::IsAnyItemHovered();

		const bool mouse_not_in_canvas = !MouseInCanvas();

		if (editor.ClickInteraction.Type != ClickInteractionType_None || any_ui_element_hovered
		    || mouse_not_in_canvas)
		{
			return;
		}

		const bool started_panning = GNodes->AltMouseClicked;

		if (started_panning)
		{
			editor.ClickInteraction.Type = ClickInteractionType_Panning;
		}
		else if (GNodes->LeftMouseClicked)
		{
			editor.ClickInteraction.Type = ClickInteractionType_BoxSelection;
			editor.ClickInteraction.BoxSelector.Rect.min =
			    ScreenSpaceToGridSpace(editor, GNodes->MousePos);
		}
	}

	void BoxSelectorUpdateSelection(EditorContext& editor, Rect box_rect)
	{
		// Invert box selector coordinates as needed

		if (box_rect.min.x > box_rect.max.x)
		{
			ImSwap(box_rect.min.x, box_rect.max.x);
		}

		if (box_rect.min.y > box_rect.max.y)
		{
			ImSwap(box_rect.min.y, box_rect.max.y);
		}

		// Update node selection

		editor.SelectedNodeIndices.clear();

		// Test for overlap against node rectangles

		for (i32 node_idx = 0; node_idx < editor.Nodes.Pool.size(); ++node_idx)
		{
			if (editor.Nodes.InUse[node_idx])
			{
				NodeData& node = editor.Nodes.Pool[node_idx];
				if (box_rect.Overlaps(node.Rect))
				{
					editor.SelectedNodeIndices.push_back(node_idx);
				}
			}
		}

		// Update link selection

		editor.SelectedLinkIndices.clear();

		// Test for overlap against links

		for (i32 link_idx = 0; link_idx < editor.Links.Pool.size(); ++link_idx)
		{
			if (editor.Links.InUse[link_idx])
			{
				const ImLinkData& link = editor.Links.Pool[link_idx];

				const PinData& pin_start    = editor.Pins.Pool[link.StartPinIdx];
				const PinData& pin_end      = editor.Pins.Pool[link.EndPinIdx];
				const Rect& node_start_rect = editor.Nodes.Pool[pin_start.ParentNodeIdx].Rect;
				const Rect& node_end_rect   = editor.Nodes.Pool[pin_end.ParentNodeIdx].Rect;

				const v2 start = GetScreenSpacePinCoordinates(
				    node_start_rect, pin_start.AttributeRect, pin_start.Type);
				const v2 end = GetScreenSpacePinCoordinates(
				    node_end_rect, pin_end.AttributeRect, pin_end.Type);

				// Test
				if (RectangleOverlapsLink(box_rect, start, end, pin_start.Type))
				{
					editor.SelectedLinkIndices.push_back(link_idx);
				}
			}
		}
	}

	v2 SnapOrigi32oGrid(v2 origin)
	{
		if ((GNodes->Style.Flags & StyleFlags_GridSnapping)
		    || ((GNodes->Style.Flags & StyleFlags_GridSnappingOnRelease)
		        && GNodes->LeftMouseReleased))
		{
			const float spacing  = GNodes->Style.GridSpacing;
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
		if (GNodes->LeftMouseDragging || GNodes->LeftMouseReleased)
		{
			const v2 origin = SnapOrigi32oGrid(GNodes->MousePos - GNodes->CanvasOriginScreenSpace
			                                   - editor.Panning + editor.PrimaryNodeOffset);
			for (i32 i = 0; i < editor.SelectedNodeIndices.size(); ++i)
			{
				const v2 node_rel  = editor.SelectedNodeOrigins[i];
				const i32 node_idx = editor.SelectedNodeIndices[i];
				NodeData& node     = editor.Nodes.Pool[node_idx];
				if (node.Draggable)
				{
					// node.Origin += io.MouseDelta - editor.AutoPanningDelta;
					node.Origin = origin + node_rel;
				}
			}
		}
	}

	struct LinkPredicate
	{
		bool operator()(const ImLinkData& lhs, const ImLinkData& rhs) const
		{
			// Do a unique compare by sorting the pins' addresses.
			// This catches duplicate links, whether they are in the
			// same direction or not.
			// Sorting by pin index should have the uniqueness guarantees as sorting
			// by id -- each unique id will get one slot in the link pool array.

			i32 lhs_start = lhs.StartPinIdx;
			i32 lhs_end   = lhs.EndPinIdx;
			i32 rhs_start = rhs.StartPinIdx;
			i32 rhs_end   = rhs.EndPinIdx;

			if (lhs_start > lhs_end)
			{
				ImSwap(lhs_start, lhs_end);
			}

			if (rhs_start > rhs_end)
			{
				ImSwap(rhs_start, rhs_end);
			}

			return lhs_start == rhs_start && lhs_end == rhs_end;
		}
	};

	ImOptionalIndex FindDuplicateLink(
	    const EditorContext& editor, const i32 start_pin_idx, const i32 end_pin_idx)
	{
		ImLinkData test_link(0);
		test_link.StartPinIdx = start_pin_idx;
		test_link.EndPinIdx   = end_pin_idx;
		for (i32 link_idx = 0; link_idx < editor.Links.Pool.size(); ++link_idx)
		{
			const ImLinkData& link = editor.Links.Pool[link_idx];
			if (LinkPredicate()(test_link, link) && editor.Links.InUse[link_idx])
			{
				return ImOptionalIndex(link_idx);
			}
		}

		return ImOptionalIndex();
	}

	bool ShouldLinkSnapToPin(const EditorContext& editor, const PinData& start_pin,
	    const i32 hovered_pin_idx, const ImOptionalIndex duplicate_link)
	{
		const PinData& end_pin = editor.Pins.Pool[hovered_pin_idx];

		// The end pin must be in a different node
		if (start_pin.ParentNodeIdx == end_pin.ParentNodeIdx)
		{
			return false;
		}

		// The end pin must be of a different type
		if (start_pin.Type == end_pin.Type)
		{
			return false;
		}

		// The link to be created must not be a duplicate, unless it is the link which was
		// created on snap. In that case we want to snap, since we want it to appear visually as
		// if the created link remains snapped to the pin.
		if (duplicate_link.HasValue() && !(duplicate_link == GNodes->SnapLinkIdx))
		{
			return false;
		}

		return true;
	}

	void ClickInteractionUpdate(EditorContext& editor)
	{
		switch (editor.ClickInteraction.Type)
		{
			case ClickInteractionType_BoxSelection: {
				editor.ClickInteraction.BoxSelector.Rect.max =
				    ScreenSpaceToGridSpace(editor, GNodes->MousePos);

				Rect box_rect = editor.ClickInteraction.BoxSelector.Rect;
				box_rect.min  = GridSpaceToScreenSpace(editor, box_rect.min);
				box_rect.max  = GridSpaceToScreenSpace(editor, box_rect.max);

				BoxSelectorUpdateSelection(editor, box_rect);

				const u32 box_selector_color   = GNodes->Style.colors[ColorVar_BoxSelector];
				const u32 box_selector_outline = GNodes->Style.colors[ColorVar_BoxSelectorOutline];
				GNodes->CanvasDrawList->AddRectFilled(
				    box_rect.min, box_rect.max, box_selector_color);
				GNodes->CanvasDrawList->AddRect(box_rect.min, box_rect.max, box_selector_outline);

				if (GNodes->LeftMouseReleased)
				{
					ImVector<i32>& depth_stack         = editor.NodeDepthOrder;
					const ImVector<i32>& selected_idxs = editor.SelectedNodeIndices;

					// Bump the selected node indices, in order, to the top of the depth stack.
					// NOTE: this algorithm has worst case time complexity of O(N^2), if the
					// node selection is ~ N (due to selected_idxs.contains()).

					if ((selected_idxs.Size > 0) && (selected_idxs.Size < depth_stack.Size))
					{
						i32 num_moved =
						    0;    // The number of indices moved. Stop after selected_idxs.Size
						for (i32 i = 0; i < depth_stack.Size - selected_idxs.Size; ++i)
						{
							for (i32 node_idx = depth_stack[i]; selected_idxs.contains(node_idx);
							     node_idx     = depth_stack[i])
							{
								depth_stack.erase(depth_stack.begin() + static_cast<size_t>(i));
								depth_stack.push_back(node_idx);
								++num_moved;
							}

							if (num_moved == selected_idxs.Size)
							{
								break;
							}
						}
					}

					editor.ClickInteraction.Type = ClickInteractionType_None;
				}
			}
			break;
			case ClickInteractionType_Node: {
				TranslateSelectedNodes(editor);

				if (GNodes->LeftMouseReleased)
				{
					editor.ClickInteraction.Type = ClickInteractionType_None;
				}
			}
			break;
			case ClickInteractionType_Link: {
				if (GNodes->LeftMouseReleased)
				{
					editor.ClickInteraction.Type = ClickInteractionType_None;
				}
			}
			break;
			case ClickInteractionType_LinkCreation: {
				const PinData& start_pin =
				    editor.Pins.Pool[editor.ClickInteraction.LinkCreation.StartPinIdx];

				const ImOptionalIndex maybe_duplicate_link_idx =
				    GNodes->HoveredPinIdx.HasValue() ? FindDuplicateLink(editor,
				        editor.ClickInteraction.LinkCreation.StartPinIdx,
				        GNodes->HoveredPinIdx.Value())
				                                     : ImOptionalIndex();

				const bool should_snap =
				    GNodes->HoveredPinIdx.HasValue()
				    && ShouldLinkSnapToPin(
				        editor, start_pin, GNodes->HoveredPinIdx.Value(), maybe_duplicate_link_idx);

				// If we created on snap and the hovered pin is empty or changed, then we need
				// signal that the link's state has changed.
				const bool snapping_pin_changed =
				    editor.ClickInteraction.LinkCreation.EndPinIdx.HasValue()
				    && !(GNodes->HoveredPinIdx == editor.ClickInteraction.LinkCreation.EndPinIdx);

				// Detach the link that was created by this link event if it's no longer in snap
				// range
				if (snapping_pin_changed && GNodes->SnapLinkIdx.HasValue())
				{
					BeginLinkDetach(editor, GNodes->SnapLinkIdx.Value(),
					    editor.ClickInteraction.LinkCreation.EndPinIdx.Value());
				}

				const v2 start_pos = GetScreenSpacePinCoordinates(editor, start_pin);
				// If we are within the hover radius of a receiving pin, snap the link
				// endpoint to it
				const v2 end_pos = should_snap ? GetScreenSpacePinCoordinates(
				                       editor, editor.Pins.Pool[GNodes->HoveredPinIdx.Value()])
				                               : GNodes->MousePos;

				const CubicBezier cubic_bezier = GetCubicBezier(
				    start_pos, end_pos, start_pin.Type, GNodes->Style.LinkLineSegmentsPerLength);
#if IMGUI_VERSION_NUM < 18000
				GNodes->CanvasDrawList->AddBezierCurve(
#else
				GNodes->CanvasDrawList->AddBezierCubic(
#endif
				    cubic_bezier.p0, cubic_bezier.p1, cubic_bezier.p2, cubic_bezier.p3,
				    GNodes->Style.colors[ColorVar_Link], GNodes->Style.LinkThickness,
				    cubic_bezier.NumSegments);

				const bool link_creation_on_snap =
				    GNodes->HoveredPinIdx.HasValue()
				    && (editor.Pins.Pool[GNodes->HoveredPinIdx.Value()].Flags
				        & AttributeFlags_EnableLinkCreationOnSnap);

				if (!should_snap)
				{
					editor.ClickInteraction.LinkCreation.EndPinIdx.Reset();
				}

				const bool create_link =
				    should_snap && (GNodes->LeftMouseReleased || link_creation_on_snap);

				if (create_link && !maybe_duplicate_link_idx.HasValue())
				{
					// Avoid send OnLinkCreated() events every frame if the snap link is not
					// saved (only applies for EnableLinkCreationOnSnap)
					if (!GNodes->LeftMouseReleased
					    && editor.ClickInteraction.LinkCreation.EndPinIdx == GNodes->HoveredPinIdx)
					{
						break;
					}

					GNodes->UIState |= UIState_LinkCreated;
					editor.ClickInteraction.LinkCreation.EndPinIdx = GNodes->HoveredPinIdx.Value();
				}

				if (GNodes->LeftMouseReleased)
				{
					editor.ClickInteraction.Type = ClickInteractionType_None;
					if (!create_link)
					{
						GNodes->UIState |= UIState_LinkDropped;
					}
				}
			}
			break;
			case ClickInteractionType_Panning: {
				const bool dragging = GNodes->AltMouseDragging;

				if (dragging)
				{
					editor.Panning += v2{ImGui::GetIO().MouseDelta};
				}
				else
				{
					editor.ClickInteraction.Type = ClickInteractionType_None;
				}
			}
			break;
			case ClickInteractionType_ImGuiItem: {
				if (GNodes->LeftMouseReleased)
				{
					editor.ClickInteraction.Type = ClickInteractionType_None;
				}
			}
			case ClickInteractionType_None: break;
			default: assert(!"Unreachable code!"); break;
		}
	}

	void ResolveOccludedPins(const EditorContext& editor, ImVector<i32>& occluded_pin_indices)
	{
		const ImVector<i32>& depth_stack = editor.NodeDepthOrder;

		occluded_pin_indices.resize(0);

		if (depth_stack.Size < 2)
		{
			return;
		}

		// For each node in the depth stack
		for (i32 depth_idx = 0; depth_idx < (depth_stack.Size - 1); ++depth_idx)
		{
			const NodeData& node_below = editor.Nodes.Pool[depth_stack[depth_idx]];

			// Iterate over the rest of the depth stack to find nodes overlapping the pins
			for (i32 next_depth_idx = depth_idx + 1; next_depth_idx < depth_stack.Size;
			     ++next_depth_idx)
			{
				const Rect& rect_above = editor.Nodes.Pool[depth_stack[next_depth_idx]].Rect;

				// Iterate over each pin
				for (i32 idx = 0; idx < node_below.PinIndices.Size; ++idx)
				{
					const i32 pin_idx = node_below.PinIndices[idx];
					const v2& pin_pos = editor.Pins.Pool[pin_idx].Pos;

					if (rect_above.Contains(pin_pos))
					{
						occluded_pin_indices.push_back(pin_idx);
					}
				}
			}
		}
	}

	ImOptionalIndex ResolveHoveredPin(
	    const ObjectPool<PinData>& pins, const ImVector<i32>& occluded_pin_indices)
	{
		float smallest_distance = FLT_MAX;
		ImOptionalIndex pin_idx_with_smallest_distance;

		const float hover_radius_sqr = GNodes->Style.PinHoverRadius * GNodes->Style.PinHoverRadius;

		for (i32 idx = 0; idx < pins.Pool.Size; ++idx)
		{
			if (!pins.InUse[idx])
			{
				continue;
			}

			if (occluded_pin_indices.contains(idx))
			{
				continue;
			}

			const v2& pin_pos        = pins.Pool[idx].Pos;
			const float distance_sqr = ImLengthSqr(pin_pos - GNodes->MousePos);

			// TODO: GNodes->Style.PinHoverRadius needs to be copied i32o pin data and the
			// pin-local value used here. This is no longer called in
			// BeginAttribute/EndAttribute scope and the detected pin might have a different
			// hover radius than what the user had when calling BeginAttribute/EndAttribute.
			if (distance_sqr < hover_radius_sqr && distance_sqr < smallest_distance)
			{
				smallest_distance              = distance_sqr;
				pin_idx_with_smallest_distance = idx;
			}
		}

		return pin_idx_with_smallest_distance;
	}

	ImOptionalIndex ResolveHoveredNode(const ImVector<i32>& depth_stack)
	{
		if (GNodes->NodeIndicesOverlappingWithMouse.size() == 0)
		{
			return ImOptionalIndex();
		}

		if (GNodes->NodeIndicesOverlappingWithMouse.size() == 1)
		{
			return ImOptionalIndex(GNodes->NodeIndicesOverlappingWithMouse[0]);
		}

		i32 largest_depth_idx = -1;
		i32 node_idx_on_top   = -1;

		for (i32 i = 0; i < GNodes->NodeIndicesOverlappingWithMouse.size(); ++i)
		{
			const i32 node_idx = GNodes->NodeIndicesOverlappingWithMouse[i];
			for (i32 depth_idx = 0; depth_idx < depth_stack.size(); ++depth_idx)
			{
				if (depth_stack[depth_idx] == node_idx && (depth_idx > largest_depth_idx))
				{
					largest_depth_idx = depth_idx;
					node_idx_on_top   = node_idx;
				}
			}
		}

		assert(node_idx_on_top != -1);
		return ImOptionalIndex(node_idx_on_top);
	}

	ImOptionalIndex ResolveHoveredLink(
	    const ObjectPool<ImLinkData>& links, const ObjectPool<PinData>& pins)
	{
		float smallest_distance = FLT_MAX;
		ImOptionalIndex link_idx_with_smallest_distance;

		// There are two ways a link can be detected as "hovered".
		// 1. The link is within hover distance to the mouse. The closest such link is selected
		// as being hovered over.
		// 2. If the link is connected to the currently hovered pin.
		//
		// The latter is a requirement for link detaching with drag click to work, as both a
		// link and pin are required to be hovered over for the feature to work.

		for (i32 idx = 0; idx < links.Pool.Size; ++idx)
		{
			if (!links.InUse[idx])
			{
				continue;
			}

			const ImLinkData& link   = links.Pool[idx];
			const PinData& start_pin = pins.Pool[link.StartPinIdx];
			const PinData& end_pin   = pins.Pool[link.EndPinIdx];

			// If there is a hovered pin links can only be considered hovered if they use that
			// pin
			if (GNodes->HoveredPinIdx.HasValue())
			{
				if (GNodes->HoveredPinIdx == link.StartPinIdx
				    || GNodes->HoveredPinIdx == link.EndPinIdx)
				{
					return idx;
				}
				continue;
			}

			// TODO: the calculated CubicBeziers could be cached since we generate them again
			// when rendering the links

			const CubicBezier cubic_bezier = GetCubicBezier(start_pin.Pos, end_pin.Pos,
			    start_pin.Type, GNodes->Style.LinkLineSegmentsPerLength);

			// The distance test
			{
				const Rect link_rect = GetContainingRectForCubicBezier(cubic_bezier);

				// First, do a simple bounding box test against the box containing the link
				// to see whether calculating the distance to the link is worth doing.
				if (link_rect.Contains(GNodes->MousePos))
				{
					const float distance = GetDistanceToCubicBezier(
					    GNodes->MousePos, cubic_bezier, cubic_bezier.NumSegments);

					// TODO: GNodes->Style.LinkHoverDistance could be also copied i32o
					// ImLinkData, since we're not calling this function in the same scope as
					// Nodes::Link(). The rendered/detected link might have a different hover
					// distance than what the user had specified when calling Link()
					if (distance < GNodes->Style.LinkHoverDistance && distance < smallest_distance)
					{
						smallest_distance               = distance;
						link_idx_with_smallest_distance = idx;
					}
				}
			}
		}

		return link_idx_with_smallest_distance;
	}

	// [SECTION] render helpers

	Rect GetItemRect()
	{
		return Rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	v2 GetNodeTitleBarOrigin(const NodeData& node)
	{
		return node.Origin + node.LayoutStyle.Padding;
	}

	v2 GetNodeContentOrigin(const NodeData& node)
	{
		const v2 title_bar_height =
		    v2(0.f, node.TitleBarContentRect.GetSize().y + 2.0f * node.LayoutStyle.Padding.y);
		return node.Origin + title_bar_height + node.LayoutStyle.Padding;
	}

	Rect GetNodeTitleRect(const NodeData& node)
	{
		Rect expanded_title_rect = node.TitleBarContentRect;
		expanded_title_rect.Expand(node.LayoutStyle.Padding);

		return Rect(expanded_title_rect.min, expanded_title_rect.min
		                                         + v2(node.Rect.GetSize().x, 0.f)
		                                         + v2(0.f, expanded_title_rect.GetSize().y));
	}

	void DrawGrid(EditorContext& editor, const v2& canvas_size)
	{
		const v2 offset     = editor.Panning;
		u32 line_color      = GNodes->Style.colors[ColorVar_GridLine];
		u32 line_color_prim = GNodes->Style.colors[ColorVar_GridLinePrimary];
		bool draw_primary   = GNodes->Style.Flags & StyleFlags_GridLinesPrimary;

		for (float x = fmodf(offset.x, GNodes->Style.GridSpacing); x < canvas_size.x;
		     x += GNodes->Style.GridSpacing)
		{
			GNodes->CanvasDrawList->AddLine(EditorSpaceToScreenSpace(v2(x, 0.0f)),
			    EditorSpaceToScreenSpace(v2(x, canvas_size.y)),
			    offset.x - x == 0.f && draw_primary ? line_color_prim : line_color);
		}

		for (float y = fmodf(offset.y, GNodes->Style.GridSpacing); y < canvas_size.y;
		     y += GNodes->Style.GridSpacing)
		{
			GNodes->CanvasDrawList->AddLine(EditorSpaceToScreenSpace(v2(0.0f, y)),
			    EditorSpaceToScreenSpace(v2(canvas_size.x, y)),
			    offset.y - y == 0.f && draw_primary ? line_color_prim : line_color);
		}
	}

	struct QuadOffsets
	{
		v2 TopLeft, BottomLeft, BottomRight, TopRight;
	};

	QuadOffsets CalculateQuadOffsets(const float side_length)
	{
		const float half_side = 0.5f * side_length;

		QuadOffsets offset;

		offset.TopLeft     = v2(-half_side, half_side);
		offset.BottomLeft  = v2(-half_side, -half_side);
		offset.BottomRight = v2(half_side, -half_side);
		offset.TopRight    = v2(half_side, half_side);

		return offset;
	}

	struct TriangleOffsets
	{
		v2 TopLeft, BottomLeft, Right;
	};

	TriangleOffsets CalculateTriangleOffsets(const float side_length)
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
		const float sqrt_3          = sqrtf(3.0f);
		const float left_offset     = -0.1666666666667f * sqrt_3 * side_length;
		const float right_offset    = 0.333333333333f * sqrt_3 * side_length;
		const float vertical_offset = 0.5f * side_length;

		TriangleOffsets offset;
		offset.TopLeft    = v2(left_offset, vertical_offset);
		offset.BottomLeft = v2(left_offset, -vertical_offset);
		offset.Right      = v2(right_offset, 0.f);

		return offset;
	}

	void DrawPinShape(const v2& pin_pos, const PinData& pin, const u32 pin_color)
	{
		static const i32 CIRCLE_NUM_SEGMENTS = 8;

		switch (pin.Shape)
		{
			case PinShape_Circle: {
				GNodes->CanvasDrawList->AddCircle(pin_pos, GNodes->Style.PinCircleRadius, pin_color,
				    CIRCLE_NUM_SEGMENTS, GNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_CircleFilled: {
				GNodes->CanvasDrawList->AddCircleFilled(
				    pin_pos, GNodes->Style.PinCircleRadius, pin_color, CIRCLE_NUM_SEGMENTS);
			}
			break;
			case PinShape_Quad: {
				const QuadOffsets offset = CalculateQuadOffsets(GNodes->Style.PinQuadSideLength);
				GNodes->CanvasDrawList->AddQuad(pin_pos + offset.TopLeft,
				    pin_pos + offset.BottomLeft, pin_pos + offset.BottomRight,
				    pin_pos + offset.TopRight, pin_color, GNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_QuadFilled: {
				const QuadOffsets offset = CalculateQuadOffsets(GNodes->Style.PinQuadSideLength);
				GNodes->CanvasDrawList->AddQuadFilled(pin_pos + offset.TopLeft,
				    pin_pos + offset.BottomLeft, pin_pos + offset.BottomRight,
				    pin_pos + offset.TopRight, pin_color);
			}
			break;
			case PinShape_Triangle: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(GNodes->Style.PinTriangleSideLength);
				GNodes->CanvasDrawList->AddTriangle(pin_pos + offset.TopLeft,
				    pin_pos + offset.BottomLeft, pin_pos + offset.Right, pin_color,
				    // NOTE: for some weird reason, the line drawn by AddTriangle is
				    // much thinner than the lines drawn by AddCircle or AddQuad.
				    // Multiplying the line thickness by two seemed to solve the
				    // problem at a few different thickness values.
				    2.f * GNodes->Style.PinLineThickness);
			}
			break;
			case PinShape_TriangleFilled: {
				const TriangleOffsets offset =
				    CalculateTriangleOffsets(GNodes->Style.PinTriangleSideLength);
				GNodes->CanvasDrawList->AddTriangleFilled(pin_pos + offset.TopLeft,
				    pin_pos + offset.BottomLeft, pin_pos + offset.Right, pin_color);
			}
			break;
			default: assert(!"Invalid PinShape value!"); break;
		}
	}

	void DrawPin(EditorContext& editor, const i32 pin_idx)
	{
		PinData& pin                 = editor.Pins.Pool[pin_idx];
		const Rect& parent_node_rect = editor.Nodes.Pool[pin.ParentNodeIdx].Rect;

		pin.Pos = GetScreenSpacePinCoordinates(parent_node_rect, pin.AttributeRect, pin.Type);

		u32 pin_color = pin.ColorStyle.Background;

		if (GNodes->HoveredPinIdx == pin_idx)
		{
			pin_color = pin.ColorStyle.Hovered;
		}

		DrawPinShape(pin.Pos, pin, pin_color);
	}

	void DrawNode(EditorContext& editor, const i32 node_idx)
	{
		const NodeData& node = editor.Nodes.Pool[node_idx];
		ImGui::SetCursorPos(node.Origin + editor.Panning);

		const bool node_hovered =
		    GNodes->HoveredNodeIdx == node_idx
		    && editor.ClickInteraction.Type != ClickInteractionType_BoxSelection;

		u32 node_background     = node.ColorStyle.Background;
		u32 titlebar_background = node.ColorStyle.Titlebar;

		if (editor.SelectedNodeIndices.contains(node_idx))
		{
			node_background     = node.ColorStyle.BackgroundSelected;
			titlebar_background = node.ColorStyle.TitlebarSelected;
		}
		else if (node_hovered)
		{
			node_background     = node.ColorStyle.BackgroundHovered;
			titlebar_background = node.ColorStyle.TitlebarHovered;
		}

		{
			// node base
			GNodes->CanvasDrawList->AddRectFilled(
			    node.Rect.min, node.Rect.max, node_background, node.LayoutStyle.CornerRounding);

			// title bar:
			if (node.TitleBarContentRect.GetSize().y > 0.f)
			{
				Rect title_bar_rect = GetNodeTitleRect(node);

				GNodes->CanvasDrawList->AddRectFilled(title_bar_rect.min, title_bar_rect.max,
				    titlebar_background, node.LayoutStyle.CornerRounding,
				    ImDrawFlags_RoundCornersTop);
			}

			if ((GNodes->Style.Flags & StyleFlags_NodeOutline) != 0
			    && node.LayoutStyle.BorderThickness > 0.f)
			{
				float halfBorder = node.LayoutStyle.BorderThickness * 0.5f;
				v2 min           = node.Rect.min;
				min.x -= halfBorder;
				min.y -= halfBorder;
				v2 max = node.Rect.max;
				max.x += halfBorder;
				max.y += halfBorder;
				GNodes->CanvasDrawList->AddRect(min, max, node.ColorStyle.Outline,
				    node.LayoutStyle.CornerRounding, ImDrawFlags_RoundCornersAll,
				    node.LayoutStyle.BorderThickness);
			}
		}

		for (i32 i = 0; i < node.PinIndices.size(); ++i)
		{
			DrawPin(editor, node.PinIndices[i]);
		}

		if (node_hovered)
		{
			GNodes->HoveredNodeIdx = node_idx;
		}
	}

	void DrawLink(EditorContext& editor, const i32 link_idx)
	{
		const ImLinkData& link   = editor.Links.Pool[link_idx];
		const PinData& start_pin = editor.Pins.Pool[link.StartPinIdx];
		const PinData& end_pin   = editor.Pins.Pool[link.EndPinIdx];

		const CubicBezier cubic_bezier = GetCubicBezier(
		    start_pin.Pos, end_pin.Pos, start_pin.Type, GNodes->Style.LinkLineSegmentsPerLength);

		const bool link_hovered =
		    GNodes->HoveredLinkIdx == link_idx
		    && editor.ClickInteraction.Type != ClickInteractionType_BoxSelection;

		if (link_hovered)
		{
			GNodes->HoveredLinkIdx = link_idx;
		}

		// It's possible for a link to be deleted in begin_link_i32eraction. A user
		// may detach a link, resulting in the link wire snapping to the mouse
		// position.
		//
		// In other words, skip rendering the link if it was deleted.
		if (GNodes->DeletedLinkIdx == link_idx)
		{
			return;
		}

		u32 link_color = link.ColorStyle.Base;
		if (editor.SelectedLinkIndices.contains(link_idx))
		{
			link_color = link.ColorStyle.Selected;
		}
		else if (link_hovered)
		{
			link_color = link.ColorStyle.Hovered;
		}

		GNodes->CanvasDrawList->AddBezierCubic(cubic_bezier.p0, cubic_bezier.p1, cubic_bezier.p2,
		    cubic_bezier.p3, link_color, GNodes->Style.LinkThickness, cubic_bezier.NumSegments);
	}

	void BeginPinAttribute(
	    const i32 id, const AttributeType type, const PinShape shape, const i32 node_idx)
	{
		// Make sure to call BeginNode() before calling
		// BeginAttribute()
		assert(GNodes->CurrentScope == Scope_Node);
		GNodes->CurrentScope = Scope_Attribute;

		ImGui::BeginGroup();
		ImGui::PushID(id);

		EditorContext& editor = EditorContextGet();

		GNodes->CurrentAttributeId = id;

		const i32 pin_idx         = ObjectPoolFindOrCreateIndex(editor.Pins, id);
		GNodes->CurrentPinIdx     = pin_idx;
		PinData& pin              = editor.Pins.Pool[pin_idx];
		pin.Id                    = id;
		pin.ParentNodeIdx         = node_idx;
		pin.Type                  = type;
		pin.Shape                 = shape;
		pin.Flags                 = GNodes->CurrentAttributeFlags;
		pin.ColorStyle.Background = GNodes->Style.colors[ColorVar_Pin];
		pin.ColorStyle.Hovered    = GNodes->Style.colors[ColorVar_PinHovered];
	}

	void EndPinAttribute()
	{
		assert(GNodes->CurrentScope == Scope_Attribute);
		GNodes->CurrentScope = Scope_Node;

		ImGui::PopID();
		ImGui::EndGroup();

		if (ImGui::IsItemActive())
		{
			GNodes->ActiveAttribute   = true;
			GNodes->ActiveAttributeId = GNodes->CurrentAttributeId;
		}

		EditorContext& editor = EditorContextGet();
		PinData& pin          = editor.Pins.Pool[GNodes->CurrentPinIdx];
		NodeData& node        = editor.Nodes.Pool[GNodes->CurrentNodeIdx];
		pin.AttributeRect     = GetItemRect();
		node.PinIndices.push_back(GNodes->CurrentPinIdx);
	}

	void Initialize(Context* context)
	{
		context->CanvasOriginScreenSpace = v2(0.0f, 0.0f);
		context->CanvasRectScreenSpace   = Rect(v2(0.f, 0.f), v2(0.f, 0.f));
		context->CurrentScope            = Scope_None;

		context->CurrentPinIdx  = INT_MAX;
		context->CurrentNodeIdx = INT_MAX;

		context->DefaultEditorCtx = EditorContextCreate();
		EditorContextSet(GNodes->DefaultEditorCtx);

		context->CurrentAttributeFlags = AttributeFlags_None;
		context->AttributeFlagStack.push_back(GNodes->CurrentAttributeFlags);

		StyleColorsDark();
	}

	void Shutdown(Context* ctx)
	{
		EditorContextFree(ctx->DefaultEditorCtx);
	}

	// [SECTION] minimap

	static bool IsMiniMapActive()
	{
		EditorContext& editor = EditorContextGet();
		return editor.miniMapEnabled && editor.miniMapSizeFraction > 0.0f;
	}

	static bool IsMiniMapHovered()
	{
		EditorContext& editor = EditorContextGet();
		return IsMiniMapActive()
		       && ImGui::IsMouseHoveringRect(
		           editor.miniMapRectScreenSpace.min, editor.miniMapRectScreenSpace.max);
	}

	static void CalcMiniMapLayout()
	{
		EditorContext& editor  = EditorContextGet();
		const v2 offset        = GNodes->Style.miniMapOffset;
		const v2 border        = GNodes->Style.miniMapPadding;
		const Rect editor_rect = GNodes->CanvasRectScreenSpace;

		// Compute the size of the mini-map area
		v2 mini_map_size;
		float mini_map_scaling;
		{
			const v2 max_size =
			    ImFloor(editor_rect.GetSize() * editor.miniMapSizeFraction - border * 2.0f);
			const float max_size_aspect_ratio     = max_size.x / max_size.y;
			const v2 grid_content_size            = editor.gridContentBounds.IsInverted()
			                                            ? max_size
			                                            : editor.gridContentBounds.GetSize().Floor();
			const float grid_content_aspect_ratio = grid_content_size.x / grid_content_size.y;
			mini_map_size    = ImFloor(grid_content_aspect_ratio > max_size_aspect_ratio
			                               ? v2(max_size.x, max_size.x / grid_content_aspect_ratio)
			                               : v2(grid_content_aspect_ratio * max_size.y, max_size.y));
			mini_map_scaling = mini_map_size.x / grid_content_size.x;
		}

		// Compute location of the mini-map
		v2 mini_map_pos;
		{
			v2 align;

			switch (editor.miniMapLocation)
			{
				case MiniMapLocation_BottomRight:
					align.x = 1.0f;
					align.y = 1.0f;
					break;
				case MiniMapLocation_BottomLeft:
					align.x = 0.0f;
					align.y = 1.0f;
					break;
				case MiniMapLocation_TopRight:
					align.x = 1.0f;
					align.y = 0.0f;
					break;
				case MiniMapLocation_TopLeft:    // [[fallthrough]]
				default:
					align.x = 0.0f;
					align.y = 0.0f;
					break;
			}

			const v2 top_left_pos     = editor_rect.min + offset + border;
			const v2 bottom_right_pos = editor_rect.max - offset - border - mini_map_size;
			mini_map_pos              = ImFloor(ImLerp(top_left_pos, bottom_right_pos, align));
		}

		editor.miniMapRectScreenSpace =
		    Rect(mini_map_pos - border, mini_map_pos + mini_map_size + border);
		editor.miniMapContentScreenSpace = Rect(mini_map_pos, mini_map_pos + mini_map_size);
		editor.miniMapScaling            = mini_map_scaling;
	}

	static void MiniMapDrawNode(EditorContext& editor, const i32 node_idx)
	{
		const NodeData& node = editor.Nodes.Pool[node_idx];

		const Rect node_rect = ScreenSpaceToMiniMapSpace(editor, node.Rect);

		// Round to near whole pixel value for corner-rounding to prevent visual glitches
		const float mini_map_node_rounding =
		    floorf(node.LayoutStyle.CornerRounding * editor.miniMapScaling);

		u32 mini_map_node_background;

		if (editor.ClickInteraction.Type == ClickInteractionType_None
		    && ImGui::IsMouseHoveringRect(node_rect.min, node_rect.max))
		{
			mini_map_node_background = GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];

			// Run user callback when hovering a mini-map node
			if (editor.miniMapNodeHoveringCallback)
			{
				editor.miniMapNodeHoveringCallback(
				    node.Id, editor.miniMapNodeHoveringCallbackUserData);
			}
		}
		else if (editor.SelectedNodeIndices.contains(node_idx))
		{
			mini_map_node_background = GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected];
		}
		else
		{
			mini_map_node_background = GNodes->Style.colors[ColorVar_MiniMapNodeBackground];
		}

		const u32 mini_map_node_outline = GNodes->Style.colors[ColorVar_MiniMapNodeOutline];

		GNodes->CanvasDrawList->AddRectFilled(
		    node_rect.min, node_rect.max, mini_map_node_background, mini_map_node_rounding);

		GNodes->CanvasDrawList->AddRect(
		    node_rect.min, node_rect.max, mini_map_node_outline, mini_map_node_rounding);
	}

	static void MiniMapDrawLink(EditorContext& editor, const i32 link_idx)
	{
		const ImLinkData& link   = editor.Links.Pool[link_idx];
		const PinData& start_pin = editor.Pins.Pool[link.StartPinIdx];
		const PinData& end_pin   = editor.Pins.Pool[link.EndPinIdx];

		const CubicBezier cubic_bezier =
		    GetCubicBezier(ScreenSpaceToMiniMapSpace(editor, start_pin.Pos),
		        ScreenSpaceToMiniMapSpace(editor, end_pin.Pos), start_pin.Type,
		        GNodes->Style.LinkLineSegmentsPerLength / editor.miniMapScaling);

		// It's possible for a link to be deleted in begin_link_i32eraction. A user
		// may detach a link, resulting in the link wire snapping to the mouse
		// position.
		//
		// In other words, skip rendering the link if it was deleted.
		if (GNodes->DeletedLinkIdx == link_idx)
		{
			return;
		}

		const u32 link_color =
		    GNodes->Style
		        .colors[editor.SelectedLinkIndices.contains(link_idx) ? ColorVar_MiniMapLinkSelected
		                                                              : ColorVar_MiniMapLink];

		GNodes->CanvasDrawList->AddBezierCubic(cubic_bezier.p0, cubic_bezier.p1, cubic_bezier.p2,
		    cubic_bezier.p3, link_color, GNodes->Style.LinkThickness * editor.miniMapScaling,
		    cubic_bezier.NumSegments);
	}

	static void MiniMapUpdate()
	{
		EditorContext& editor = EditorContextGet();

		u32 mini_map_background;

		if (IsMiniMapHovered())
		{
			mini_map_background = GNodes->Style.colors[ColorVar_MiniMapBackgroundHovered];
		}
		else
		{
			mini_map_background = GNodes->Style.colors[ColorVar_MiniMapBackground];
		}

		// Create a child window bellow mini-map, so it blocks all mouse i32eraction on canvas.
		i32 flags = ImGuiWindowFlags_NoBackground;
		ImGui::SetCursorScreenPos(editor.miniMapRectScreenSpace.min);
		ImGui::BeginChild("minimap", editor.miniMapRectScreenSpace.GetSize(), false, flags);

		const Rect& mini_map_rect = editor.miniMapRectScreenSpace;

		// Draw minimap background and border
		GNodes->CanvasDrawList->AddRectFilled(
		    mini_map_rect.min, mini_map_rect.max, mini_map_background);

		GNodes->CanvasDrawList->AddRect(
		    mini_map_rect.min, mini_map_rect.max, GNodes->Style.colors[ColorVar_MiniMapOutline]);

		// Clip draw list items to mini-map rect (after drawing background/outline)
		GNodes->CanvasDrawList->PushClipRect(
		    mini_map_rect.min, mini_map_rect.max, true /* i32ersect with editor clip-rect */);

		// Draw links first so they appear under nodes, and we can use the same draw channel
		for (i32 link_idx = 0; link_idx < editor.Links.Pool.size(); ++link_idx)
		{
			if (editor.Links.InUse[link_idx])
			{
				MiniMapDrawLink(editor, link_idx);
			}
		}

		for (i32 node_idx = 0; node_idx < editor.Nodes.Pool.size(); ++node_idx)
		{
			if (editor.Nodes.InUse[node_idx])
			{
				MiniMapDrawNode(editor, node_idx);
			}
		}

		// Draw editor canvas rect inside mini-map
		{
			const u32 canvas_color  = GNodes->Style.colors[ColorVar_MiniMapCanvas];
			const u32 outline_color = GNodes->Style.colors[ColorVar_MiniMapCanvasOutline];
			const Rect rect = ScreenSpaceToMiniMapSpace(editor, GNodes->CanvasRectScreenSpace);

			GNodes->CanvasDrawList->AddRectFilled(rect.min, rect.max, canvas_color);
			GNodes->CanvasDrawList->AddRect(rect.min, rect.max, outline_color);
		}

		// Have to pop mini-map clip rect
		GNodes->CanvasDrawList->PopClipRect();

		bool mini_map_is_hovered = ImGui::IsWindowHovered();

		ImGui::EndChild();

		bool center_on_click = mini_map_is_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)
		                       && editor.ClickInteraction.Type == ClickInteractionType_None
		                       && !GNodes->NodeIdxSubmissionOrder.empty();
		if (center_on_click)
		{
			v2 target      = MiniMapSpaceToGridSpace(editor, ImGui::GetMousePos());
			v2 center      = GNodes->CanvasRectScreenSpace.GetSize() * 0.5f;
			editor.Panning = ImFloor(center - target);
		}

		// Reset callback info after use
		editor.miniMapNodeHoveringCallback         = nullptr;
		editor.miniMapNodeHoveringCallbackUserData = nullptr;
	}

	// [SECTION] selection helpers

	template<typename T>
	void SelectObject(const ObjectPool<T>& objects, ImVector<i32>& selected_indices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		assert(idx >= 0);
		assert(selected_indices.find(idx) == selected_indices.end());
		selected_indices.push_back(idx);
	}

	template<typename T>
	void ClearObjectSelection(
	    const ObjectPool<T>& objects, ImVector<i32>& selected_indices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		assert(idx >= 0);
		assert(selected_indices.find(idx) != selected_indices.end());
		selected_indices.find_erase_unsorted(idx);
	}

	template<typename T>
	bool IsObjectSelected(
	    const ObjectPool<T>& objects, ImVector<i32>& selected_indices, const i32 id)
	{
		const i32 idx = ObjectPoolFind(objects, id);
		return selected_indices.find(idx) != selected_indices.end();
	}


	// [SECTION] API implementation

	IO::EmulateThreeButtonMouse::EmulateThreeButtonMouse() : Modifier(nullptr) {}

	IO::LinkDetachWithModifierClick::LinkDetachWithModifierClick() : Modifier(nullptr) {}

	IO::MultipleSelectModifier::MultipleSelectModifier() : Modifier(nullptr) {}

	IO::IO()
	    : EmulateThreeButtonMouse()
	    , LinkDetachWithModifierClick()
	    , AltMouseButton(ImGuiMouseButton_Middle)
	    , AutoPanningSpeed(1000.0f)
	{}

	Style::Style()
	    : GridSpacing(32.f)
	    , NodeCornerRounding(4.f)
	    , NodePadding(8.f, 8.f)
	    , NodeBorderThickness(1.f)
	    , LinkThickness(3.f)
	    , LinkLineSegmentsPerLength(0.1f)
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
		Context* ctx = IM_NEW(Context)();
		if (GNodes == nullptr)
			SetCurrentContext(ctx);
		Initialize(ctx);
		return ctx;
	}

	void DestroyContext(Context* ctx)
	{
		if (ctx == nullptr)
			ctx = GNodes;
		Shutdown(ctx);
		if (GNodes == ctx)
			SetCurrentContext(nullptr);
		IM_DELETE(ctx);
	}

	Context* GetCurrentContext()
	{
		return GNodes;
	}

	void SetCurrentContext(Context* ctx)
	{
		GNodes = ctx;
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
		GNodes->EditorCtx = ctx;
	}

	v2 EditorContextGetPanning()
	{
		const EditorContext& editor = EditorContextGet();
		return editor.Panning;
	}

	void EditorContextResetPanning(const v2& pos)
	{
		EditorContext& editor = EditorContextGet();
		editor.Panning        = pos;
	}

	void EditorContextMoveToNode(const i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.Nodes, node_id);

		editor.Panning.x = -node.Origin.x;
		editor.Panning.y = -node.Origin.y;
	}

	void SetImGuiContext(ImGuiContext* ctx)
	{
		ImGui::SetCurrentContext(ctx);
	}

	IO& GetIO()
	{
		return GNodes->Io;
	}

	Style& GetStyle()
	{
		return GNodes->Style;
	}

	void StyleColorsDark()
	{
		GNodes->Style.colors[ColorVar_NodeBackground]         = IM_COL32(50, 50, 50, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = IM_COL32(75, 75, 75, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundSelected] = IM_COL32(75, 75, 75, 255);
		GNodes->Style.colors[ColorVar_NodeOutline]            = IM_COL32(100, 100, 100, 255);
		// title bar colors match ImGui's titlebg colors
		GNodes->Style.colors[ColorVar_TitleBar]         = IM_COL32(41, 74, 122, 255);
		GNodes->Style.colors[ColorVar_TitleBarHovered]  = IM_COL32(66, 150, 250, 255);
		GNodes->Style.colors[ColorVar_TitleBarSelected] = IM_COL32(66, 150, 250, 255);
		// link colors match ImGui's slider grab colors
		GNodes->Style.colors[ColorVar_Link]         = IM_COL32(61, 133, 224, 200);
		GNodes->Style.colors[ColorVar_LinkHovered]  = IM_COL32(66, 150, 250, 255);
		GNodes->Style.colors[ColorVar_LinkSelected] = IM_COL32(66, 150, 250, 255);
		// pin colors match ImGui's button colors
		GNodes->Style.colors[ColorVar_Pin]        = IM_COL32(53, 150, 250, 180);
		GNodes->Style.colors[ColorVar_PinHovered] = IM_COL32(53, 150, 250, 255);

		GNodes->Style.colors[ColorVar_BoxSelector]        = IM_COL32(61, 133, 224, 30);
		GNodes->Style.colors[ColorVar_BoxSelectorOutline] = IM_COL32(61, 133, 224, 150);

		GNodes->Style.colors[ColorVar_GridBackground] = IM_COL32(40, 40, 50, 200);
		GNodes->Style.colors[ColorVar_GridLine]       = IM_COL32(200, 200, 200, 40);

		GNodes->Style.colors[ColorVar_GridLinePrimary] = IM_COL32(240, 240, 240, 60);

		// minimap colors
		GNodes->Style.colors[ColorVar_MiniMapBackground]            = IM_COL32(25, 25, 25, 150);
		GNodes->Style.colors[ColorVar_MiniMapBackgroundHovered]     = IM_COL32(25, 25, 25, 200);
		GNodes->Style.colors[ColorVar_MiniMapOutline]               = IM_COL32(150, 150, 150, 100);
		GNodes->Style.colors[ColorVar_MiniMapOutlineHovered]        = IM_COL32(150, 150, 150, 200);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackground]        = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered] = IM_COL32(200, 200, 200, 255);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		GNodes->Style.colors[ColorVar_MiniMapNodeOutline] = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapLink]        = GNodes->Style.colors[ColorVar_Link];
		GNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    GNodes->Style.colors[ColorVar_LinkSelected];
		GNodes->Style.colors[ColorVar_MiniMapCanvas]        = IM_COL32(200, 200, 200, 25);
		GNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = IM_COL32(200, 200, 200, 200);
	}

	void StyleColorsClassic()
	{
		GNodes->Style.colors[ColorVar_NodeBackground]         = IM_COL32(50, 50, 50, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = IM_COL32(75, 75, 75, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundSelected] = IM_COL32(75, 75, 75, 255);
		GNodes->Style.colors[ColorVar_NodeOutline]            = IM_COL32(100, 100, 100, 255);
		GNodes->Style.colors[ColorVar_TitleBar]               = IM_COL32(69, 69, 138, 255);
		GNodes->Style.colors[ColorVar_TitleBarHovered]        = IM_COL32(82, 82, 161, 255);
		GNodes->Style.colors[ColorVar_TitleBarSelected]       = IM_COL32(82, 82, 161, 255);
		GNodes->Style.colors[ColorVar_Link]                   = IM_COL32(255, 255, 255, 100);
		GNodes->Style.colors[ColorVar_LinkHovered]            = IM_COL32(105, 99, 204, 153);
		GNodes->Style.colors[ColorVar_LinkSelected]           = IM_COL32(105, 99, 204, 153);
		GNodes->Style.colors[ColorVar_Pin]                    = IM_COL32(89, 102, 156, 170);
		GNodes->Style.colors[ColorVar_PinHovered]             = IM_COL32(102, 122, 179, 200);
		GNodes->Style.colors[ColorVar_BoxSelector]            = IM_COL32(82, 82, 161, 100);
		GNodes->Style.colors[ColorVar_BoxSelectorOutline]     = IM_COL32(82, 82, 161, 255);
		GNodes->Style.colors[ColorVar_GridBackground]         = IM_COL32(40, 40, 50, 200);
		GNodes->Style.colors[ColorVar_GridLine]               = IM_COL32(200, 200, 200, 40);
		GNodes->Style.colors[ColorVar_GridLinePrimary]        = IM_COL32(240, 240, 240, 60);

		// minimap colors
		GNodes->Style.colors[ColorVar_MiniMapBackground]        = IM_COL32(25, 25, 25, 100);
		GNodes->Style.colors[ColorVar_MiniMapBackgroundHovered] = IM_COL32(25, 25, 25, 200);
		GNodes->Style.colors[ColorVar_MiniMapOutline]           = IM_COL32(150, 150, 150, 100);
		GNodes->Style.colors[ColorVar_MiniMapOutlineHovered]    = IM_COL32(150, 150, 150, 200);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackground]    = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] = IM_COL32(200, 200, 240, 255);
		GNodes->Style.colors[ColorVar_MiniMapNodeOutline]            = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapLink] = GNodes->Style.colors[ColorVar_Link];
		GNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    GNodes->Style.colors[ColorVar_LinkSelected];
		GNodes->Style.colors[ColorVar_MiniMapCanvas]        = IM_COL32(200, 200, 200, 25);
		GNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = IM_COL32(200, 200, 200, 200);
	}

	void StyleColorsLight()
	{
		GNodes->Style.colors[ColorVar_NodeBackground]         = IM_COL32(240, 240, 240, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundHovered]  = IM_COL32(240, 240, 240, 255);
		GNodes->Style.colors[ColorVar_NodeBackgroundSelected] = IM_COL32(240, 240, 240, 255);
		GNodes->Style.colors[ColorVar_NodeOutline]            = IM_COL32(100, 100, 100, 255);
		GNodes->Style.colors[ColorVar_TitleBar]               = IM_COL32(248, 248, 248, 255);
		GNodes->Style.colors[ColorVar_TitleBarHovered]        = IM_COL32(209, 209, 209, 255);
		GNodes->Style.colors[ColorVar_TitleBarSelected]       = IM_COL32(209, 209, 209, 255);
		// original imgui values: 66, 150, 250
		GNodes->Style.colors[ColorVar_Link] = IM_COL32(66, 150, 250, 100);
		// original imgui values: 117, 138, 204
		GNodes->Style.colors[ColorVar_LinkHovered]  = IM_COL32(66, 150, 250, 242);
		GNodes->Style.colors[ColorVar_LinkSelected] = IM_COL32(66, 150, 250, 242);
		// original imgui values: 66, 150, 250
		GNodes->Style.colors[ColorVar_Pin]                = IM_COL32(66, 150, 250, 160);
		GNodes->Style.colors[ColorVar_PinHovered]         = IM_COL32(66, 150, 250, 255);
		GNodes->Style.colors[ColorVar_BoxSelector]        = IM_COL32(90, 170, 250, 30);
		GNodes->Style.colors[ColorVar_BoxSelectorOutline] = IM_COL32(90, 170, 250, 150);
		GNodes->Style.colors[ColorVar_GridBackground]     = IM_COL32(225, 225, 225, 255);
		GNodes->Style.colors[ColorVar_GridLine]           = IM_COL32(180, 180, 180, 100);
		GNodes->Style.colors[ColorVar_GridLinePrimary]    = IM_COL32(120, 120, 120, 100);

		// minimap colors
		GNodes->Style.colors[ColorVar_MiniMapBackground]        = IM_COL32(25, 25, 25, 100);
		GNodes->Style.colors[ColorVar_MiniMapBackgroundHovered] = IM_COL32(25, 25, 25, 200);
		GNodes->Style.colors[ColorVar_MiniMapOutline]           = IM_COL32(150, 150, 150, 100);
		GNodes->Style.colors[ColorVar_MiniMapOutlineHovered]    = IM_COL32(150, 150, 150, 200);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackground]    = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] =
		    GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundHovered];
		GNodes->Style.colors[ColorVar_MiniMapNodeBackgroundSelected] = IM_COL32(200, 200, 240, 255);
		GNodes->Style.colors[ColorVar_MiniMapNodeOutline]            = IM_COL32(200, 200, 200, 100);
		GNodes->Style.colors[ColorVar_MiniMapLink] = GNodes->Style.colors[ColorVar_Link];
		GNodes->Style.colors[ColorVar_MiniMapLinkSelected] =
		    GNodes->Style.colors[ColorVar_LinkSelected];
		GNodes->Style.colors[ColorVar_MiniMapCanvas]        = IM_COL32(200, 200, 200, 25);
		GNodes->Style.colors[ColorVar_MiniMapCanvasOutline] = IM_COL32(200, 200, 200, 200);
	}

	void BeginNodeEditor()
	{
		assert(GNodes->CurrentScope == Scope_None);
		GNodes->CurrentScope = Scope_Editor;

		// Reset state from previous pass

		EditorContext& editor    = EditorContextGet();
		editor.AutoPanningDelta  = v2(0, 0);
		editor.gridContentBounds = Rect(v2{FLT_MAX, FLT_MAX}, v2{FLT_MIN, FLT_MIN});
		editor.miniMapEnabled    = false;
		ObjectPoolReset(editor.Nodes);
		ObjectPoolReset(editor.Pins);
		ObjectPoolReset(editor.Links);

		GNodes->HoveredNodeIdx.Reset();
		GNodes->HoveredLinkIdx.Reset();
		GNodes->HoveredPinIdx.Reset();
		GNodes->DeletedLinkIdx.Reset();
		GNodes->SnapLinkIdx.Reset();

		GNodes->NodeIndicesOverlappingWithMouse.clear();

		GNodes->UIState = UIState_None;

		GNodes->MousePos          = ImGui::GetIO().MousePos;
		GNodes->LeftMouseClicked  = ImGui::IsMouseClicked(0);
		GNodes->LeftMouseReleased = ImGui::IsMouseReleased(0);
		GNodes->LeftMouseDragging = ImGui::IsMouseDragging(0, 0.0f);
		GNodes->AltMouseClicked =
		    (GNodes->Io.EmulateThreeButtonMouse.Modifier != nullptr
		        && *GNodes->Io.EmulateThreeButtonMouse.Modifier && GNodes->LeftMouseClicked)
		    || ImGui::IsMouseClicked(GNodes->Io.AltMouseButton);
		GNodes->AltMouseDragging =
		    (GNodes->Io.EmulateThreeButtonMouse.Modifier != nullptr && GNodes->LeftMouseDragging
		        && (*GNodes->Io.EmulateThreeButtonMouse.Modifier))
		    || ImGui::IsMouseDragging(GNodes->Io.AltMouseButton, 0.0f);
		GNodes->AltMouseScrollDelta    = ImGui::GetIO().MouseWheel;
		GNodes->MultipleSelectModifier = (GNodes->Io.MultipleSelectModifier.Modifier != nullptr
		                                      ? *GNodes->Io.MultipleSelectModifier.Modifier
		                                      : ImGui::GetIO().KeyCtrl);
		GNodes->ActiveAttribute        = false;

		ImGui::BeginGroup();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, v2(1.f, 1.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, v2(0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, GNodes->Style.colors[ColorVar_GridBackground]);
			ImGui::BeginChild("scrolling_region", v2(0.f, 0.f), true,
			    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove
			        | ImGuiWindowFlags_NoScrollWithMouse);
			GNodes->CanvasOriginScreenSpace = ImGui::GetCursorScreenPos();

			// NOTE: we have to fetch the canvas draw list *after* we call
			// BeginChild(), otherwise the ImGui UI elements are going to be
			// rendered i32o the parent window draw list.
			DrawListSet(ImGui::GetWindowDrawList());

			{
				const v2 canvas_size          = ImGui::GetWindowSize();
				GNodes->CanvasRectScreenSpace = Rect(
				    EditorSpaceToScreenSpace(v2(0.f, 0.f)), EditorSpaceToScreenSpace(canvas_size));

				if (GNodes->Style.Flags & StyleFlags_GridLines)
				{
					DrawGrid(editor, canvas_size);
				}
			}
		}
	}

	void EndNodeEditor()
	{
		assert(GNodes->CurrentScope == Scope_Editor);
		GNodes->CurrentScope = Scope_None;

		EditorContext& editor = EditorContextGet();

		bool no_grid_content = editor.gridContentBounds.IsInverted();
		if (no_grid_content)
		{
			editor.gridContentBounds =
			    ScreenSpaceToGridSpace(editor, GNodes->CanvasRectScreenSpace);
		}

		// Detect ImGui i32eraction first, because it blocks i32eraction with the rest of the UI

		if (GNodes->LeftMouseClicked && ImGui::IsAnyItemActive())
		{
			editor.ClickInteraction.Type = ClickInteractionType_ImGuiItem;
		}

		// Detect which UI element is being hovered over. Detection is done in a hierarchical
		// fashion, because a UI element being hovered excludes any other as being hovered over.

		// Don't do hovering detection for nodes/links/pins when i32eracting with the mini-map,
		// since its an *overlay* with its own i32eraction behavior and must have precedence during
		// mouse i32eraction.

		if ((editor.ClickInteraction.Type == ClickInteractionType_None
		        || editor.ClickInteraction.Type == ClickInteractionType_LinkCreation)
		    && MouseInCanvas() && !IsMiniMapHovered())
		{
			// Pins needs some special care. We need to check the depth stack to see which pins are
			// being occluded by other nodes.
			ResolveOccludedPins(editor, GNodes->OccludedPinIndices);

			GNodes->HoveredPinIdx = ResolveHoveredPin(editor.Pins, GNodes->OccludedPinIndices);

			if (!GNodes->HoveredPinIdx.HasValue())
			{
				// Resolve which node is actually on top and being hovered using the depth stack.
				GNodes->HoveredNodeIdx = ResolveHoveredNode(editor.NodeDepthOrder);
			}

			// We don't check for hovered pins here, because if we want to detach a link by clicking
			// and dragging, we need to have both a link and pin hovered.
			if (!GNodes->HoveredNodeIdx.HasValue())
			{
				GNodes->HoveredLinkIdx = ResolveHoveredLink(editor.Links, editor.Pins);
			}
		}

		for (i32 node_idx = 0; node_idx < editor.Nodes.Pool.size(); ++node_idx)
		{
			if (editor.Nodes.InUse[node_idx])
			{
				DrawListActivateNodeBackground(node_idx);
				DrawNode(editor, node_idx);
			}
		}

		// In order to render the links underneath the nodes, we want to first select the bottom
		// draw channel.
		GNodes->CanvasDrawList->ChannelsSetCurrent(0);

		for (i32 link_idx = 0; link_idx < editor.Links.Pool.size(); ++link_idx)
		{
			if (editor.Links.InUse[link_idx])
			{
				DrawLink(editor, link_idx);
			}
		}

		// Render the click i32eraction UI elements (partial links, box selector) on top of
		// everything else.

		DrawListAppendClickInteractionChannel();
		DrawListActivateClickInteractionChannel();

		if (IsMiniMapActive())
		{
			CalcMiniMapLayout();
			MiniMapUpdate();
		}

		// Handle node graph i32eraction

		if (!IsMiniMapHovered())
		{
			if (GNodes->LeftMouseClicked && GNodes->HoveredLinkIdx.HasValue())
			{
				BeginLinki32eraction(editor, GNodes->HoveredLinkIdx.Value(), GNodes->HoveredPinIdx);
			}

			else if (GNodes->LeftMouseClicked && GNodes->HoveredPinIdx.HasValue())
			{
				BeginLinkCreation(editor, GNodes->HoveredPinIdx.Value());
			}

			else if (GNodes->LeftMouseClicked && GNodes->HoveredNodeIdx.HasValue())
			{
				BeginNodeSelection(editor, GNodes->HoveredNodeIdx.Value());
			}

			else if (GNodes->LeftMouseClicked || GNodes->LeftMouseReleased
			         || GNodes->AltMouseClicked || GNodes->AltMouseScrollDelta != 0.f)
			{
				BeginCanvasi32eraction(editor);
			}

			bool should_auto_pan =
			    editor.ClickInteraction.Type == ClickInteractionType_BoxSelection
			    || editor.ClickInteraction.Type == ClickInteractionType_LinkCreation
			    || editor.ClickInteraction.Type == ClickInteractionType_Node;
			if (should_auto_pan && !MouseInCanvas())
			{
				v2 mouse     = ImGui::GetMousePos();
				v2 center    = GNodes->CanvasRectScreenSpace.GetCenter();
				v2 direction = (center - mouse);
				direction    = direction * ImInvLength(direction, 0.0);

				editor.AutoPanningDelta =
				    direction * ImGui::GetIO().DeltaTime * GNodes->Io.AutoPanningSpeed;
				editor.Panning += editor.AutoPanningDelta;
			}
		}
		ClickInteractionUpdate(editor);

		// At this point, draw commands have been issued for all nodes (and pins). Update the node
		// pool to detect unused node slots and remove those indices from the depth stack before
		// sorting the node draw commands by depth.
		ObjectPoolUpdate(editor.Nodes);
		ObjectPoolUpdate(editor.Pins);

		DrawListSortChannelsByDepth(editor.NodeDepthOrder);

		// After the links have been rendered, the link pool can be updated as well.
		ObjectPoolUpdate(editor.Links);

		// Finally, merge the draw channels
		GNodes->CanvasDrawList->ChannelsMerge();

		// pop style
		ImGui::EndChild();         // end scrolling region
		ImGui::PopStyleColor();    // pop child window background color
		ImGui::PopStyleVar();      // pop window padding
		ImGui::PopStyleVar();      // pop frame padding
		ImGui::EndGroup();
	}

	void MiniMap(const float minimap_size_fraction, const MiniMapLocation location,
	    const MiniMapNodeHoveringCallback node_hovering_callback,
	    const MiniMapNodeHoveringCallbackUserData node_hovering_callback_data)
	{
		// Check that editor size fraction is sane; must be in the range (0, 1]
		assert(minimap_size_fraction > 0.f && minimap_size_fraction <= 1.f);

		// Remember to call before EndNodeEditor
		assert(GNodes->CurrentScope == Scope_Editor);

		EditorContext& editor = EditorContextGet();

		editor.miniMapEnabled      = true;
		editor.miniMapSizeFraction = minimap_size_fraction;
		editor.miniMapLocation     = location;

		// Set node hovering callback information
		editor.miniMapNodeHoveringCallback         = node_hovering_callback;
		editor.miniMapNodeHoveringCallbackUserData = node_hovering_callback_data;

		// Actual drawing/updating of the MiniMap is done in EndNodeEditor so that
		// mini map is draw over everything and all pin/link positions are updated
		// correctly relative to their respective nodes. Hence, we must store some of
		// of the state for the mini map in GNodes for the actual drawing/updating
	}

	void BeginNode(const i32 node_id)
	{
		// Remember to call BeginNodeEditor before calling BeginNode
		assert(GNodes->CurrentScope == Scope_Editor);
		GNodes->CurrentScope = Scope_Node;

		EditorContext& editor = EditorContextGet();

		const i32 node_idx     = ObjectPoolFindOrCreateIndex(editor.Nodes, node_id);
		GNodes->CurrentNodeIdx = node_idx;

		NodeData& node                     = editor.Nodes.Pool[node_idx];
		node.ColorStyle.Background         = GNodes->Style.colors[ColorVar_NodeBackground];
		node.ColorStyle.BackgroundHovered  = GNodes->Style.colors[ColorVar_NodeBackgroundHovered];
		node.ColorStyle.BackgroundSelected = GNodes->Style.colors[ColorVar_NodeBackgroundSelected];
		node.ColorStyle.Outline            = GNodes->Style.colors[ColorVar_NodeOutline];
		node.ColorStyle.Titlebar           = GNodes->Style.colors[ColorVar_TitleBar];
		node.ColorStyle.TitlebarHovered    = GNodes->Style.colors[ColorVar_TitleBarHovered];
		node.ColorStyle.TitlebarSelected   = GNodes->Style.colors[ColorVar_TitleBarSelected];
		node.LayoutStyle.CornerRounding    = GNodes->Style.NodeCornerRounding;
		node.LayoutStyle.Padding           = GNodes->Style.NodePadding;
		node.LayoutStyle.BorderThickness   = GNodes->Style.NodeBorderThickness;

		// ImGui::SetCursorPos sets the cursor position, local to the current widget
		// (in this case, the child object started in BeginNodeEditor). Use
		// ImGui::SetCursorScreenPos to set the screen space coordinates directly.
		ImGui::SetCursorPos(GridSpaceToEditorSpace(editor, GetNodeTitleBarOrigin(node)));

		DrawListAddNode(node_idx);
		DrawListActivateCurrentNodeForeground();

		ImGui::PushID(node.Id);
		ImGui::BeginGroup();
	}

	void EndNode()
	{
		assert(GNodes->CurrentScope == Scope_Node);
		GNodes->CurrentScope = Scope_Editor;

		EditorContext& editor = EditorContextGet();

		// The node's rectangle depends on the ImGui UI group size.
		ImGui::EndGroup();
		ImGui::PopID();

		NodeData& node = editor.Nodes.Pool[GNodes->CurrentNodeIdx];
		node.Rect      = GetItemRect();
		node.Rect.Expand(node.LayoutStyle.Padding);

		editor.gridContentBounds.Add(node.Origin);
		editor.gridContentBounds.Add(node.Origin + node.Rect.GetSize());

		if (node.Rect.Contains(GNodes->MousePos))
		{
			GNodes->NodeIndicesOverlappingWithMouse.push_back(GNodes->CurrentNodeIdx);
		}
	}

	v2 GetNodeDimensions(i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		const i32 node_idx    = ObjectPoolFind(editor.Nodes, node_id);
		assert(node_idx != -1);    // invalid node_id
		const NodeData& node = editor.Nodes.Pool[node_idx];
		return node.Rect.GetSize();
	}

	void BeginNodeTitleBar()
	{
		assert(GNodes->CurrentScope == Scope_Node);
		ImGui::BeginGroup();
	}

	void EndNodeTitleBar()
	{
		assert(GNodes->CurrentScope == Scope_Node);
		ImGui::EndGroup();

		EditorContext& editor    = EditorContextGet();
		NodeData& node           = editor.Nodes.Pool[GNodes->CurrentNodeIdx];
		node.TitleBarContentRect = GetItemRect();

		Rect nodeTitleRect = GetNodeTitleRect(node);
		ImGui::ItemAdd({nodeTitleRect.min, nodeTitleRect.max}, ImGui::GetID("title_bar"));

		ImGui::SetCursorPos(GridSpaceToEditorSpace(editor, GetNodeContentOrigin(node)));
	}

	void BeginInputAttribute(const i32 id, const PinShape shape)
	{
		BeginPinAttribute(id, AttributeType_Input, shape, GNodes->CurrentNodeIdx);
	}

	void EndInputAttribute()
	{
		EndPinAttribute();
	}

	void BeginOutputAttribute(const i32 id, const PinShape shape)
	{
		BeginPinAttribute(id, AttributeType_Output, shape, GNodes->CurrentNodeIdx);
	}

	void EndOutputAttribute()
	{
		EndPinAttribute();
	}

	void BeginStaticAttribute(const i32 id)
	{
		// Make sure to call BeginNode() before calling BeginAttribute()
		assert(GNodes->CurrentScope == Scope_Node);
		GNodes->CurrentScope = Scope_Attribute;

		GNodes->CurrentAttributeId = id;

		ImGui::BeginGroup();
		ImGui::PushID(id);
	}

	void EndStaticAttribute()
	{
		// Make sure to call BeginNode() before calling BeginAttribute()
		assert(GNodes->CurrentScope == Scope_Attribute);
		GNodes->CurrentScope = Scope_Node;

		ImGui::PopID();
		ImGui::EndGroup();

		if (ImGui::IsItemActive())
		{
			GNodes->ActiveAttribute   = true;
			GNodes->ActiveAttributeId = GNodes->CurrentAttributeId;
		}
	}

	void PushAttributeFlag(const AttributeFlags flag)
	{
		GNodes->CurrentAttributeFlags |= flag;
		GNodes->AttributeFlagStack.push_back(GNodes->CurrentAttributeFlags);
	}

	void PopAttributeFlag()
	{
		// PopAttributeFlag called without a matching PushAttributeFlag!
		// The bottom value is always the default value, pushed in Initialize().
		assert(GNodes->AttributeFlagStack.size() > 1);

		GNodes->AttributeFlagStack.pop_back();
		GNodes->CurrentAttributeFlags = GNodes->AttributeFlagStack.back();
	}

	void Link(const i32 id, const i32 start_attr_id, const i32 end_attr_id)
	{
		assert(GNodes->CurrentScope == Scope_Editor);

		EditorContext& editor    = EditorContextGet();
		ImLinkData& link         = ObjectPoolFindOrCreateObject(editor.Links, id);
		link.Id                  = id;
		link.StartPinIdx         = ObjectPoolFindOrCreateIndex(editor.Pins, start_attr_id);
		link.EndPinIdx           = ObjectPoolFindOrCreateIndex(editor.Pins, end_attr_id);
		link.ColorStyle.Base     = GNodes->Style.colors[ColorVar_Link];
		link.ColorStyle.Hovered  = GNodes->Style.colors[ColorVar_LinkHovered];
		link.ColorStyle.Selected = GNodes->Style.colors[ColorVar_LinkSelected];

		// Check if this link was created by the current link event
		if ((editor.ClickInteraction.Type == ClickInteractionType_LinkCreation
		        && editor.Pins.Pool[link.EndPinIdx].Flags & AttributeFlags_EnableLinkCreationOnSnap
		        && editor.ClickInteraction.LinkCreation.StartPinIdx == link.StartPinIdx
		        && editor.ClickInteraction.LinkCreation.EndPinIdx == link.EndPinIdx)
		    || (editor.ClickInteraction.LinkCreation.StartPinIdx == link.EndPinIdx
		        && editor.ClickInteraction.LinkCreation.EndPinIdx == link.StartPinIdx))
		{
			GNodes->SnapLinkIdx = ObjectPoolFindOrCreateIndex(editor.Links, id);
		}
	}

	void PushColorStyle(const ColorVar item, u32 color)
	{
		GNodes->ColorModifierStack.push_back(ColElement(item, GNodes->Style.colors[item]));
		GNodes->Style.colors[item] = color;
	}

	void PopColorStyle()
	{
		assert(GNodes->ColorModifierStack.size() > 0);
		const ColElement elem           = GNodes->ColorModifierStack.back();
		GNodes->Style.colors[elem.item] = elem.color;
		GNodes->ColorModifierStack.pop_back();
	}

	struct StyleVarInfo
	{
		ImGuiDataType Type;
		u32 Count;
		u32 Offset;
		void* GetVarPtr(Style* style) const
		{
			return (void*)((u8*)style + Offset);
		}
	};

	static const StyleVarInfo GStyleVarInfo[] = {
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
 // StyleVar_LinkLineSegmentsPerLength
	    {ImGuiDataType_Float, 1, (u32)IM_OFFSETOF(Style, LinkLineSegmentsPerLength)},
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
 // StyleVar_miniMapOffset
	    {ImGuiDataType_Float, 2, (u32)IM_OFFSETOF(Style, miniMapOffset)            },
	};

	static const StyleVarInfo* GetStyleVarInfo(StyleVar idx)
	{
		IM_ASSERT(idx >= 0 && idx < StyleVar_COUNT);
		IM_ASSERT(IM_ARRAYSIZE(GStyleVarInfo) == StyleVar_COUNT);
		return &GStyleVarInfo[idx];
	}

	void PushStyleVar(const StyleVar item, const float value)
	{
		const StyleVarInfo* var_info = GetStyleVarInfo(item);
		if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1)
		{
			float& style_var = *(float*)var_info->GetVarPtr(&GNodes->Style);
			GNodes->StyleModifierStack.push_back(StyleVarElement(item, style_var));
			style_var = value;
			return;
		}
		IM_ASSERT(0 && "Called PushStyleVar() float variant but variable is not a float!");
	}

	void PushStyleVar(const StyleVar item, const v2& value)
	{
		const StyleVarInfo* var_info = GetStyleVarInfo(item);
		if (var_info->Type == ImGuiDataType_Float && var_info->Count == 2)
		{
			v2& style_var = *(v2*)var_info->GetVarPtr(&GNodes->Style);
			GNodes->StyleModifierStack.push_back(StyleVarElement(item, style_var));
			style_var = value;
			return;
		}
		IM_ASSERT(0 && "Called PushStyleVar() v2 variant but variable is not a v2!");
	}

	void PopStyleVar(i32 count)
	{
		while (count > 0)
		{
			assert(GNodes->StyleModifierStack.size() > 0);
			const StyleVarElement style_backup = GNodes->StyleModifierStack.back();
			GNodes->StyleModifierStack.pop_back();
			const StyleVarInfo* var_info = GetStyleVarInfo(style_backup.item);
			void* style_var              = var_info->GetVarPtr(&GNodes->Style);
			if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1)
			{
				((float*)style_var)[0] = style_backup.value[0];
			}
			else if (var_info->Type == ImGuiDataType_Float && var_info->Count == 2)
			{
				((float*)style_var)[0] = style_backup.value[0];
				((float*)style_var)[1] = style_backup.value[1];
			}
			count--;
		}
	}

	void SetNodeScreenSpacePos(const i32 node_id, const v2& screen_space_pos)
	{
		EditorContext& editor = EditorContextGet();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.Nodes, node_id);
		node.Origin           = ScreenSpaceToGridSpace(editor, screen_space_pos);
	}

	void SetNodeEditorSpacePos(const i32 node_id, const v2& editor_space_pos)
	{
		EditorContext& editor = EditorContextGet();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.Nodes, node_id);
		node.Origin           = EditorSpaceToGridSpace(editor, editor_space_pos);
	}

	void SetNodeGridSpacePos(const i32 node_id, const v2& grid_pos)
	{
		EditorContext& editor = EditorContextGet();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.Nodes, node_id);
		node.Origin           = grid_pos;
	}

	void SetNodeDraggable(const i32 node_id, const bool draggable)
	{
		EditorContext& editor = EditorContextGet();
		NodeData& node        = ObjectPoolFindOrCreateObject(editor.Nodes, node_id);
		node.Draggable        = draggable;
	}

	v2 GetNodeScreenSpacePos(const i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		const i32 node_idx    = ObjectPoolFind(editor.Nodes, node_id);
		assert(node_idx != -1);
		NodeData& node = editor.Nodes.Pool[node_idx];
		return GridSpaceToScreenSpace(editor, node.Origin);
	}

	v2 GetNodeEditorSpacePos(const i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		const i32 node_idx    = ObjectPoolFind(editor.Nodes, node_id);
		assert(node_idx != -1);
		NodeData& node = editor.Nodes.Pool[node_idx];
		return GridSpaceToEditorSpace(editor, node.Origin);
	}

	v2 GetNodeGridSpacePos(const i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		const i32 node_idx    = ObjectPoolFind(editor.Nodes, node_id);
		assert(node_idx != -1);
		NodeData& node = editor.Nodes.Pool[node_idx];
		return node.Origin;
	}

	bool IsEditorHovered()
	{
		return MouseInCanvas();
	}

	bool IsNodeHovered(i32* const node_id)
	{
		assert(GNodes->CurrentScope != Scope_None);
		assert(node_id != nullptr);

		const bool is_hovered = GNodes->HoveredNodeIdx.HasValue();
		if (is_hovered)
		{
			const EditorContext& editor = EditorContextGet();
			*node_id                    = editor.Nodes.Pool[GNodes->HoveredNodeIdx.Value()].Id;
		}
		return is_hovered;
	}

	bool IsLinkHovered(i32* const link_id)
	{
		assert(GNodes->CurrentScope != Scope_None);
		assert(link_id != nullptr);

		const bool is_hovered = GNodes->HoveredLinkIdx.HasValue();
		if (is_hovered)
		{
			const EditorContext& editor = EditorContextGet();
			*link_id                    = editor.Links.Pool[GNodes->HoveredLinkIdx.Value()].Id;
		}
		return is_hovered;
	}

	bool IsPinHovered(i32* const attr)
	{
		assert(GNodes->CurrentScope != Scope_None);
		assert(attr != nullptr);

		const bool is_hovered = GNodes->HoveredPinIdx.HasValue();
		if (is_hovered)
		{
			const EditorContext& editor = EditorContextGet();
			*attr                       = editor.Pins.Pool[GNodes->HoveredPinIdx.Value()].Id;
		}
		return is_hovered;
	}

	i32 NumSelectedNodes()
	{
		assert(GNodes->CurrentScope != Scope_None);
		const EditorContext& editor = EditorContextGet();
		return editor.SelectedNodeIndices.size();
	}

	i32 NumSelectedLinks()
	{
		assert(GNodes->CurrentScope != Scope_None);
		const EditorContext& editor = EditorContextGet();
		return editor.SelectedLinkIndices.size();
	}

	void GetSelectedNodes(i32* node_ids)
	{
		assert(node_ids != nullptr);

		const EditorContext& editor = EditorContextGet();
		for (i32 i = 0; i < editor.SelectedNodeIndices.size(); ++i)
		{
			const i32 node_idx = editor.SelectedNodeIndices[i];
			node_ids[i]        = editor.Nodes.Pool[node_idx].Id;
		}
	}

	void GetSelectedLinks(i32* link_ids)
	{
		assert(link_ids != nullptr);

		const EditorContext& editor = EditorContextGet();
		for (i32 i = 0; i < editor.SelectedLinkIndices.size(); ++i)
		{
			const i32 link_idx = editor.SelectedLinkIndices[i];
			link_ids[i]        = editor.Links.Pool[link_idx].Id;
		}
	}

	void ClearNodeSelection()
	{
		EditorContext& editor = EditorContextGet();
		editor.SelectedNodeIndices.clear();
	}

	void ClearNodeSelection(i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		ClearObjectSelection(editor.Nodes, editor.SelectedNodeIndices, node_id);
	}

	void ClearLinkSelection()
	{
		EditorContext& editor = EditorContextGet();
		editor.SelectedLinkIndices.clear();
	}

	void ClearLinkSelection(i32 link_id)
	{
		EditorContext& editor = EditorContextGet();
		ClearObjectSelection(editor.Links, editor.SelectedLinkIndices, link_id);
	}

	void SelectNode(i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		SelectObject(editor.Nodes, editor.SelectedNodeIndices, node_id);
	}

	void SelectLink(i32 link_id)
	{
		EditorContext& editor = EditorContextGet();
		SelectObject(editor.Links, editor.SelectedLinkIndices, link_id);
	}

	bool IsNodeSelected(i32 node_id)
	{
		EditorContext& editor = EditorContextGet();
		return IsObjectSelected(editor.Nodes, editor.SelectedNodeIndices, node_id);
	}

	bool IsLinkSelected(i32 link_id)
	{
		EditorContext& editor = EditorContextGet();
		return IsObjectSelected(editor.Links, editor.SelectedLinkIndices, link_id);
	}

	bool IsAttributeActive()
	{
		assert((GNodes->CurrentScope & Scope_Node) != 0);

		if (!GNodes->ActiveAttribute)
		{
			return false;
		}

		return GNodes->ActiveAttributeId == GNodes->CurrentAttributeId;
	}

	bool IsAnyAttributeActive(i32* const attribute_id)
	{
		assert((GNodes->CurrentScope & (Scope_Node | Scope_Attribute)) == 0);

		if (!GNodes->ActiveAttribute)
		{
			return false;
		}

		if (attribute_id != nullptr)
		{
			*attribute_id = GNodes->ActiveAttributeId;
		}

		return true;
	}

	bool IsLinkStarted(i32* const started_at_id)
	{
		// Call this function after EndNodeEditor()!
		assert(GNodes->CurrentScope != Scope_None);
		assert(started_at_id != nullptr);

		const bool is_started = (GNodes->UIState & UIState_LinkStarted) != 0;
		if (is_started)
		{
			const EditorContext& editor = EditorContextGet();
			const i32 pin_idx           = editor.ClickInteraction.LinkCreation.StartPinIdx;
			*started_at_id              = editor.Pins.Pool[pin_idx].Id;
		}

		return is_started;
	}

	bool IsLinkDropped(i32* const started_at_id, const bool including_detached_links)
	{
		// Call this function after EndNodeEditor()!
		assert(GNodes->CurrentScope != Scope_None);

		const EditorContext& editor = EditorContextGet();

		const bool link_dropped =
		    (GNodes->UIState & UIState_LinkDropped) != 0
		    && (including_detached_links
		        || editor.ClickInteraction.LinkCreation.Type != LinkCreationType_FromDetach);

		if (link_dropped && started_at_id)
		{
			const i32 pin_idx = editor.ClickInteraction.LinkCreation.StartPinIdx;
			*started_at_id    = editor.Pins.Pool[pin_idx].Id;
		}

		return link_dropped;
	}

	bool IsLinkCreated(
	    i32* const started_at_pin_id, i32* const ended_at_pin_id, bool* const created_from_snap)
	{
		assert(GNodes->CurrentScope != Scope_None);
		assert(started_at_pin_id != nullptr);
		assert(ended_at_pin_id != nullptr);

		const bool is_created = (GNodes->UIState & UIState_LinkCreated) != 0;

		if (is_created)
		{
			const EditorContext& editor = EditorContextGet();
			const i32 start_idx         = editor.ClickInteraction.LinkCreation.StartPinIdx;
			const i32 end_idx           = editor.ClickInteraction.LinkCreation.EndPinIdx.Value();
			const PinData& start_pin    = editor.Pins.Pool[start_idx];
			const PinData& end_pin      = editor.Pins.Pool[end_idx];

			if (start_pin.Type == AttributeType_Output)
			{
				*started_at_pin_id = start_pin.Id;
				*ended_at_pin_id   = end_pin.Id;
			}
			else
			{
				*started_at_pin_id = end_pin.Id;
				*ended_at_pin_id   = start_pin.Id;
			}

			if (created_from_snap)
			{
				*created_from_snap =
				    editor.ClickInteraction.Type == ClickInteractionType_LinkCreation;
			}
		}

		return is_created;
	}

	bool IsLinkCreated(i32* started_at_node_id, i32* started_at_pin_id, i32* ended_at_node_id,
	    i32* ended_at_pin_id, bool* created_from_snap)
	{
		assert(GNodes->CurrentScope != Scope_None);
		assert(started_at_node_id != nullptr);
		assert(started_at_pin_id != nullptr);
		assert(ended_at_node_id != nullptr);
		assert(ended_at_pin_id != nullptr);

		const bool is_created = (GNodes->UIState & UIState_LinkCreated) != 0;

		if (is_created)
		{
			const EditorContext& editor = EditorContextGet();
			const i32 start_idx         = editor.ClickInteraction.LinkCreation.StartPinIdx;
			const i32 end_idx           = editor.ClickInteraction.LinkCreation.EndPinIdx.Value();
			const PinData& start_pin    = editor.Pins.Pool[start_idx];
			const NodeData& start_node  = editor.Nodes.Pool[start_pin.ParentNodeIdx];
			const PinData& end_pin      = editor.Pins.Pool[end_idx];
			const NodeData& end_node    = editor.Nodes.Pool[end_pin.ParentNodeIdx];

			if (start_pin.Type == AttributeType_Output)
			{
				*started_at_pin_id  = start_pin.Id;
				*started_at_node_id = start_node.Id;
				*ended_at_pin_id    = end_pin.Id;
				*ended_at_node_id   = end_node.Id;
			}
			else
			{
				*started_at_pin_id  = end_pin.Id;
				*started_at_node_id = end_node.Id;
				*ended_at_pin_id    = start_pin.Id;
				*ended_at_node_id   = start_node.Id;
			}

			if (created_from_snap)
			{
				*created_from_snap =
				    editor.ClickInteraction.Type == ClickInteractionType_LinkCreation;
			}
		}

		return is_created;
	}

	bool IsLinkDestroyed(i32* const link_id)
	{
		assert(GNodes->CurrentScope != Scope_None);

		const bool link_destroyed = GNodes->DeletedLinkIdx.HasValue();
		if (link_destroyed)
		{
			const EditorContext& editor = EditorContextGet();
			const i32 link_idx          = GNodes->DeletedLinkIdx.Value();
			*link_id                    = editor.Links.Pool[link_idx].Id;
		}

		return link_destroyed;
	}
}    // namespace Rift::Nodes
