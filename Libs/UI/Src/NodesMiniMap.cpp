// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/NodesMiniMap.h"

#include "UI/Nodes.h"
#include "UI/NodesInternal.h"

#include <Math/Math.h>
#include <Math/Vector.h>


namespace Rift::Nodes
{
	bool MiniMap::IsActive() const
	{
		return enabled && sizeFraction > 0.0f;
	}

	bool MiniMap::IsHovered() const
	{
		return IsActive() && ImGui::IsMouseHoveringRect(rectScreenSpace.min, rectScreenSpace.max);
	}

	void MiniMap::CalculateLayout()
	{
		EditorContext& editor = GetEditorContext();

		const v2 offset       = gNodes->style.miniMapOffset;
		const v2 border       = gNodes->style.miniMapPadding;
		const Rect editorRect = gNodes->CanvasRectScreenSpace;

		// Compute the size of the mini-map area
		v2 miniMapSize;
		float miniMapScaling;
		{
			const v2 maxSize = (editorRect.GetSize() * sizeFraction - border * 2.0f).Floor();
			const float maxSizeAspectRatio     = maxSize.x / maxSize.y;
			const v2 gridContentSize           = editor.gridContentBounds.IsInverted()
			                                       ? maxSize
			                                       : editor.gridContentBounds.GetSize().Floor();
			const float gridContentAspectRatio = gridContentSize.x / gridContentSize.y;

			if (gridContentAspectRatio > maxSizeAspectRatio)
			{
				miniMapSize = v2(maxSize.x, maxSize.x / gridContentAspectRatio).Floor();
			}
			else
			{
				miniMapSize = v2(gridContentAspectRatio * maxSize.y, maxSize.y).Floor();
			}
			miniMapScaling = miniMapSize.x / gridContentSize.x;
		}

		// Compute location of the mini-map
		v2 miniMapPos;
		{
			v2 align;

			switch (corner)
			{
				case MiniMapCorner::BottomRight:
					align.x = 1.0f;
					align.y = 1.0f;
					break;
				case MiniMapCorner::BottomLeft:
					align.x = 0.0f;
					align.y = 1.0f;
					break;
				case MiniMapCorner::TopRight:
					align.x = 1.0f;
					align.y = 0.0f;
					break;
				case MiniMapCorner::TopLeft:
				default:
					align.x = 0.0f;
					align.y = 0.0f;
					break;
			}

			const v2 topLeftPos     = editorRect.min + offset + border;
			const v2 bottomRightPos = editorRect.max - offset - border - miniMapSize;
			miniMapPos              = ImFloor(ImLerp(topLeftPos, bottomRightPos, align));
		}

		rectScreenSpace    = Rect(miniMapPos - border, miniMapPos + miniMapSize + border);
		contentScreenSpace = Rect(miniMapPos, miniMapPos + miniMapSize);
		scaling            = miniMapScaling;
	}

	void MiniMap::DrawNode(EditorContext& editor, const i32 nodeIdx)
	{
		const NodeData& node = editor.nodes.Pool[nodeIdx];

		const Rect nodeRect{ScreenToMiniMapPosition(editor, node.rect.min),
		    ScreenToMiniMapPosition(editor, node.rect.max)};

		// Round to near whole pixel value for corner-rounding to prevent visual glitches
		const float miniMapNodeRounding = Math::Floor(node.LayoutStyle.CornerRounding * scaling);

		Color miniMapNodeBackground;
		if (editor.clickInteraction.type == ClickInteractionType_None
		    && ImGui::IsMouseHoveringRect(nodeRect.min, nodeRect.max))
		{
			miniMapNodeBackground = gNodes->style.colors[ColorVar_MiniMapNodeBackgroundHovered];

			// Run user callback when hovering a mini-map node
			if (nodeHoveringCallback)
			{
				nodeHoveringCallback(node.id, nodeHoveringCallbackUserData);
			}
		}
		else if (editor.SelectedNodeIndices.contains(nodeIdx))
		{
			miniMapNodeBackground = gNodes->style.colors[ColorVar_MiniMapNodeBackgroundSelected];
		}
		else
		{
			miniMapNodeBackground = gNodes->style.colors[ColorVar_MiniMapNodeBackground];
		}

		const Color miniMapNodeOutline = gNodes->style.colors[ColorVar_MiniMapNodeOutline];

		gNodes->CanvasDrawList->AddRectFilled(
		    nodeRect.min, nodeRect.max, miniMapNodeBackground.ToPackedABGR(), miniMapNodeRounding);

		gNodes->CanvasDrawList->AddRect(
		    nodeRect.min, nodeRect.max, miniMapNodeOutline.ToPackedABGR(), miniMapNodeRounding);
	}

	void MiniMap::DrawLink(EditorContext& editor, const i32 linkIdx)
	{
		const LinkData& link      = editor.Links.Pool[linkIdx];
		const PinData& outputData = editor.outputs.Pool[link.outputIdx];
		const PinData& inputData  = editor.inputs.Pool[link.inputIdx];

		const v2 outputPosition       = ScreenToMiniMapPosition(editor, outputData.position);
		const v2 inputPosition        = ScreenToMiniMapPosition(editor, inputData.position);
		const CubicBezier cubicBezier = MakeCubicBezier(outputPosition, inputPosition,
		    PinType::Output, gNodes->style.linkLineSegmentsPerLength / scaling);

		// It's possible for a link to be deleted in begin_link_i32eraction. A user
		// may detach a link, resulting in the link wire snapping to the mouse
		// position.
		//
		// In other words, skip rendering the link if it was deleted.
		if (gNodes->DeletedLinkIdx == linkIdx)
		{
			return;
		}

		const Color linkColor =
		    gNodes->style
		        .colors[editor.SelectedLinkIndices.contains(linkIdx) ? ColorVar_MiniMapLinkSelected
		                                                             : ColorVar_MiniMapLink];

		gNodes->CanvasDrawList->AddBezierCubic(cubicBezier.p0, cubicBezier.p1, cubicBezier.p2,
		    cubicBezier.p3, linkColor.ToPackedABGR(), gNodes->style.LinkThickness * scaling,
		    cubicBezier.numSegments);
	}

	void MiniMap::Update()
	{
		EditorContext& editor = GetEditorContext();

		Color miniMapBackground;
		if (IsHovered())
		{
			miniMapBackground = gNodes->style.colors[ColorVar_MiniMapBackgroundHovered];
		}
		else
		{
			miniMapBackground = gNodes->style.colors[ColorVar_MiniMapBackground];
		}

		// Create a child window bellow mini-map, so it blocks all mouse i32eraction on canvas.
		i32 flags = ImGuiWindowFlags_NoBackground;
		ImGui::SetCursorScreenPos(rectScreenSpace.min);
		ImGui::BeginChild("minimap", rectScreenSpace.GetSize(), false, flags);

		const Rect& miniMapRect = rectScreenSpace;

		// Draw minimap background and border
		gNodes->CanvasDrawList->AddRectFilled(
		    miniMapRect.min, miniMapRect.max, miniMapBackground.ToPackedABGR());

		gNodes->CanvasDrawList->AddRect(miniMapRect.min, miniMapRect.max,
		    gNodes->style.colors[ColorVar_MiniMapOutline].ToPackedABGR());

		// Clip draw list items to mini-map rect (after drawing background/outline)
		gNodes->CanvasDrawList->PushClipRect(
		    miniMapRect.min, miniMapRect.max, true /* i32ersect with editor clip-rect */);

		// Draw links first so they appear under nodes, and we can use the same draw channel
		for (i32 linkIdx = 0; linkIdx < editor.Links.Pool.Size(); ++linkIdx)
		{
			if (editor.Links.InUse.IsSet(linkIdx))
			{
				DrawLink(editor, linkIdx);
			}
		}

		for (i32 nodeIdx = 0; nodeIdx < editor.nodes.Pool.Size(); ++nodeIdx)
		{
			if (editor.nodes.InUse.IsSet(nodeIdx))
			{
				DrawNode(editor, nodeIdx);
			}
		}

		// Draw editor canvas rect inside mini-map
		{
			const Color canvasColor  = gNodes->style.colors[ColorVar_MiniMapCanvas];
			const Color outlineColor = gNodes->style.colors[ColorVar_MiniMapCanvasOutline];
			const Rect rect{ScreenToMiniMapPosition(editor, gNodes->CanvasRectScreenSpace.min),
			    ScreenToMiniMapPosition(editor, gNodes->CanvasRectScreenSpace.max)};

			gNodes->CanvasDrawList->AddRectFilled(rect.min, rect.max, canvasColor.ToPackedABGR());
			gNodes->CanvasDrawList->AddRect(rect.min, rect.max, outlineColor.ToPackedABGR());
		}

		// Have to pop mini-map clip rect
		gNodes->CanvasDrawList->PopClipRect();

		bool miniMapIsHovered = ImGui::IsWindowHovered();

		ImGui::EndChild();

		const bool centerOnClick = miniMapIsHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)
		                        && editor.clickInteraction.type == ClickInteractionType_None
		                        && !gNodes->NodeIdxSubmissionOrder.empty();
		if (centerOnClick)
		{
			v2 target      = MiniMapToGridPosition(editor, ImGui::GetMousePos());
			v2 center      = gNodes->CanvasRectScreenSpace.GetSize() * 0.5f;
			editor.Panning = ImFloor(center - target);
		}

		// Reset callback info after use
		nodeHoveringCallback         = nullptr;
		nodeHoveringCallbackUserData = nullptr;
	}

	void DrawMiniMap(const float minimapSizeFraction, const MiniMapCorner corner,
	    const MiniMap::NodeHoveringCallback nodeHoveringCallback,
	    const MiniMap::NodeHoveringCallbackUserData nodeHoveringCallbackData)
	{
		// Check that editor size fraction is sane; must be in the range (0, 1]
		assert(minimapSizeFraction > 0.f && minimapSizeFraction <= 1.f);

		// Remember to call before EndNodeEditor
		assert(gNodes->currentScope == Scope::Editor);

		EditorContext& editor = GetEditorContext();

		MiniMap& miniMap     = editor.miniMap;
		miniMap.enabled      = true;
		miniMap.sizeFraction = minimapSizeFraction;
		miniMap.corner       = corner;

		// Set node hovering callback information
		miniMap.nodeHoveringCallback         = nodeHoveringCallback;
		miniMap.nodeHoveringCallbackUserData = nodeHoveringCallbackData;

		// Actual drawing/updating of the MiniMap is done in EndNodeEditor so that
		// mini map is draw over everything and all pin/link positions are updated
		// correctly relative to their respective nodes. Hence, we must store some of
		// of the state for the mini map in gNodes for the actual drawing/updating
	}
}    // namespace Rift::Nodes
