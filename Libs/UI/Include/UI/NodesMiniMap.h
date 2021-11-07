// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include <Platform/Platform.h>


namespace Rift::Nodes
{
	enum class MiniMapCorner : u8
	{
		BottomLeft,
		BottomRight,
		TopLeft,
		TopRight,
	};

	struct MiniMap
	{
		using NodeHoveringCallback         = void (*)(i32, void*);
		using NodeHoveringCallbackUserData = void*;


		// Mini-map state set by MiniMap()
		bool enabled                                              = false;
		MiniMapCorner corner                                      = MiniMapCorner::TopLeft;
		float sizeFraction                                        = 0.f;
		NodeHoveringCallback nodeHoveringCallback                 = nullptr;
		NodeHoveringCallbackUserData nodeHoveringCallbackUserData = nullptr;

		// Mini-map state set during EndNodeEditor() call
		Rect rectScreenSpace;
		Rect contentScreenSpace;
		float scaling = 0.f;

		bool IsActive() const;
		bool IsHovered() const;
		void CalculateLayout();

		void DrawNode(EditorContext& editor, const i32 nodeIdx);
		void DrawLink(EditorContext& editor, const i32 linkIdx);
		void Update();
	};

	// Add a navigable minimap to the editor; call before EndNodeEditor after all
	// nodes and links have been specified
	void DrawMiniMap(float minimapSizeFraction                         = 0.2f,
	    MiniMapCorner corner                                           = MiniMapCorner::TopLeft,
	    MiniMap::NodeHoveringCallback nodeHoveringCallback             = nullptr,
	    MiniMap::NodeHoveringCallbackUserData nodeHoveringCallbackData = nullptr);
}    // namespace Rift::Nodes
