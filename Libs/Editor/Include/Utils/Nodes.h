// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <AST/Id.h>
#include <Pipe/Core/EnumFlags.h>
#include <PipeColor.h>
#include <PipeMath.h>
#include <UI/UIImgui.h>


#ifdef IMNODES_USER_CONFIG
	#include IMNODES_USER_CONFIG
#endif

struct ImGuiContext;


namespace rift::Nodes
{
	using namespace p;


	using Id = i32;
	constexpr Id NoId()
	{
		return 0;
	}

	using StyleVar   = i32;    // -> enum StyleVar_
	using StyleFlags = i32;    // -> enum StyleFlags_
	using PinShape   = i32;    // -> enum PinShape_
	using PinFlags   = i32;    // -> enum PinFlags_

	enum ColorVar : u8
	{
		ColorVar_NodeBackground = 0,
		ColorVar_NodeBackgroundHovered,
		ColorVar_NodeBackgroundSelected,
		ColorVar_NodeOutline,
		ColorVar_TitleBar,
		ColorVar_TitleBarHovered,
		ColorVar_TitleBarSelected,
		ColorVar_Link,
		ColorVar_LinkHovered,
		ColorVar_LinkSelected,
		ColorVar_Pin,
		ColorVar_PinHovered,
		ColorVar_BoxSelector,
		ColorVar_BoxSelectorOutline,
		ColorVar_GridBackground,
		ColorVar_GridLine,
		ColorVar_GridLinePrimary,
		ColorVar_MiniMapBackground,
		ColorVar_MiniMapBackgroundHovered,
		ColorVar_MiniMapOutline,
		ColorVar_MiniMapOutlineHovered,
		ColorVar_MiniMapNodeBackground,
		ColorVar_MiniMapNodeBackgroundHovered,
		ColorVar_MiniMapNodeBackgroundSelected,
		ColorVar_MiniMapNodeOutline,
		ColorVar_MiniMapLink,
		ColorVar_MiniMapLinkSelected,
		ColorVar_MiniMapCanvas,
		ColorVar_MiniMapCanvasOutline,
		ColorVar_COUNT
	};

	enum StyleVar_
	{
		StyleVar_GridSpacing = 0,
		StyleVar_NodeCornerRounding,
		StyleVar_NodePadding,
		StyleVar_NodeBorderThickness,
		StyleVar_LinkThickness,
		StyleVar_linkLineSegmentsPerLength,
		StyleVar_LinkHoverDistance,
		StyleVar_PinCircleRadius,
		StyleVar_PinQuadSideLength,
		StyleVar_PinTriangleSideLength,
		StyleVar_PinDiamondSideLength,
		StyleVar_PinLineThickness,
		StyleVar_PinHoverRadius,
		StyleVar_PinOffset,
		StyleVar_MiniMapPadding,
		StyleVar_MiniMapOffset,
		StyleVar_COUNT
	};

	enum StyleFlags_
	{
		StyleFlags_None                  = 0,
		StyleFlags_NodeOutline           = 1 << 0,
		StyleFlags_GridLines             = 1 << 2,
		StyleFlags_GridLinesPrimary      = 1 << 3,
		StyleFlags_GridSnapping          = 1 << 4,
		StyleFlags_GridSnappingOnRelease = 1 << 5
	};

	enum PinShape_
	{
		PinShape_Circle,
		PinShape_CircleFilled,
		PinShape_Triangle,
		PinShape_TriangleFilled,
		PinShape_Quad,
		PinShape_QuadFilled,
		PinShape_Diamond,
		PinShape_DiamondFilled
	};

	enum class PinType : u8
	{
		None = 0,
		Input,
		Output,
		Max
	};


	struct PinIdx
	{
		i32 index    = NO_INDEX;
		PinType type = PinType::None;


		static constexpr PinIdx Output(i32 index)
		{
			return {index, PinType::Output};
		}
		static constexpr PinIdx Input(i32 index)
		{
			return {index, PinType::Input};
		}
		static constexpr PinIdx Invalid()
		{
			return {};
		}

		bool operator==(PinIdx other) const
		{
			return index == other.index && type == other.type;
		}
		operator bool() const
		{
			return index != NO_INDEX && type != PinType::None;
		}
		operator i32() const
		{
			return index;
		}
	};


	enum PinFlags_
	{
		PinFlags_None = 0,
		// A link can be detached by dragging it from the pin. The link still has to be deleted
		PinFlags_EnableLinkDetachWithDragClick = 1 << 0,
		// Dragging a link over a pin previews the link it would create
		PinFlags_EnableLinkCreationOnSnap = 1 << 1
	};

	struct IO
	{
		struct EmulateThreeButtonMouse
		{
			// Modifier that pans the editor while the left mouse button is held
			const bool* modifier = nullptr;
		} emulateThreeButtonMouse;

		struct LinkDetachWithModifierClick
		{
			// Modifier that detaches a link when it is left-clicked
			const bool* modifier = nullptr;
		} linkDetachWithModifierClick;

		struct MultipleSelectModifier
		{
			// Modifier that adds a clicked node to the selection. Ctrl when null
			const bool* modifier = nullptr;
		} multipleSelectModifier;

		// Mouse button that pans the node editor, as an ImGuiMouseButton value
		i32 AltMouseButton = ImGuiMouseButton_Middle;

		// Panning speed when dragging an element with the mouse outside the editor view
		float AutoPanningSpeed = 1000.f;
	};

	struct Style
	{
		float GridSpacing;

		float NodeCornerRounding;
		v2 NodePadding;
		float NodeBorderThickness;

		float LinkThickness;
		float linkLineSegmentsPerLength;
		float LinkHoverDistance;

		// Pin shapes are sized to occupy a similar surface area on screen
		float PinCircleRadius;
		float PinQuadSideLength;
		float PinTriangleSideLength;
		float PinDiamondSideLength;
		// Thickness of the line of unfilled pins
		float PinLineThickness;
		// Radius around a pin that counts as hovering it
		float PinHoverRadius;
		// Offset of the pins outside the node
		float PinOffset;

		v2 miniMapPadding{8.f, 8.f};
		v2 miniMapOffset{4.f, 4.f};

		StyleFlags Flags;
		// Indexed by ColorVar_*
		Color colors[u8(ColorVar_COUNT)];

		Style();
	};

	struct CubicBezier
	{
		v2 p0, p1, p2, p3;
		i32 numSegments;
	};

	struct Context;

	// The nodes of a single workspace, one Begin/EndNodeEditor pair. One is created by default
	struct EditorContext;


	EditorContext& GetEditorContext();

	CubicBezier MakeCubicBezier(
	    v2 start, v2 end, const PinType startType, const float lineSegmentsPerLength);

	// Call this function if you are compiling UI in to a dll, separate from ImGui. Calling
	// this function sets the GImGui global variable, which is not shared across dll
	// boundaries.
	void SetImGuiContext(ImGuiContext* ctx);

	Context* CreateContext();
	void DestroyContext(Context* ctx = nullptr);    // nullptr = destroy current context
	Context* GetCurrentContext();
	void SetCurrentContext(Context* ctx);

	EditorContext* EditorContextCreate();
	void EditorContextFree(EditorContext*);
	void SetEditorContext(EditorContext*);
	v2 GetPanning();
	void ResetPanning(const v2& pos);
	void MoveToNode(ast::Id nodeId, v2 offset = v2::Zero());

	// Canvas zoom, read from and written to the editor settings. Zooming keeps the point
	// under the mouse cursor fixed. Use ResetZoom() to return to the default zoom.
	float GetZoom();
	void SetZoom(float zoom);
	void ResetZoom();

	IO& GetIO();

	// Returns the global style struct. See the struct declaration for default values.
	Style& GetStyle();
	// Style presets matching the dear imgui styles of the same name.
	void StyleColorsDark();    // on by default
	void StyleColorsClassic();
	void StyleColorsLight();

	// The top-level function call. Call this before calling BeginNode/EndNode. Calling this
	// function will result the node editor grid workspace being rendered.
	void BeginNodeEditor();
	void EndNodeEditor();


	v2 EditorToScreenPosition(const v2& v);
	v2 ScreenToGridPosition(const EditorContext& editor, const v2& v);
	v2 GridToScreenPosition(const EditorContext& editor, const v2& v);
	v2 GridToEditorPosition(const EditorContext& editor, const v2& v);
	v2 EditorToGridPosition(const EditorContext& editor, const v2& v);
	v2 MiniMapToGridPosition(const EditorContext& editor, const v2& v);
	v2 ScreenToMiniMapPosition(const EditorContext& editor, const v2& v);
	v2 ScreenToGridPosition(const v2& v);
	v2 GridToScreenPosition(const v2& v);
	v2 GridToEditorPosition(const v2& v);
	v2 EditorToGridPosition(const v2& v);
	v2 MiniMapToGridPosition(const v2& v);
	v2 ScreenToMiniMapPosition(const v2& v);


	void PushStyleColor(ColorVar item, Color color);
	void PopStyleColor(i32 count = 1);
	void PushStyleVar(StyleVar style_item, float value);
	void PushStyleVar(StyleVar style_item, const v2& value);
	void PopStyleVar(i32 count = 1);

	// id can be any positive or negative integer, but INT_MIN is reserved for internal use.
	// Content is placed by column: BeginInput() blocks in the inputs, BeginOutput() blocks in
	// the outputs and anything else in the name. Rows and columns without content collapse.
	void BeginNode(ast::Id id);
	void EndNode();

	v2 GetNodeDimensions(ast::Id id);

	// The title bar fills the header row. Draw it before adding any pins
	void BeginTitleBar();
	void EndTitleBar();

	// Each pin id must be unique

	// Create an input attribute block. The pin is rendered on left side.
	void BeginInput(Id id, PinShape shape = PinShape_CircleFilled);
	void EndInput();
	// Create an output attribute block. The pin is rendered on the right side.
	void BeginOutput(Id id, PinShape shape = PinShape_CircleFilled);
	void EndOutput();

	void PushPinFlag(PinFlags flag);
	void PopPinFlag();

	// The pins must match the ids used by Begin(Input|Output)
	void Link(Id id, Id outputPin, Id inputPin);

	// Enable or disable the ability to click and drag a specific node.
	void SetNodeDraggable(Id nodeId, const bool draggable);

	// Node positions can be addressed in screen, editor and grid space
	void SetNodeScreenSpacePos(ast::Id nodeId, const v2& screen_space_pos);
	void SetNodeEditorSpacePos(ast::Id nodeId, const v2& editor_space_pos);
	void SetNodeGridSpacePos(ast::Id nodeId, const v2& grid_pos);

	v2 GetNodeScreenSpacePos(ast::Id nodeId);
	v2 GetNodeEditorSpacePos(ast::Id nodeId);
	v2 GetNodeGridSpacePos(ast::Id nodeId);

	// True when the canvas is hovered and no other window is blocking the mouse
	bool IsEditorHovered();
	ast::Id GetHoveredNode();
	ast::Id GetHoveredLink();

	bool IsNodeHovered(ast::Id nodeId);
	bool IsLinkHovered(ast::Id linkId);
	bool IsPinHovered(Id* attributeId);

	// Returns the on-screen position and color of a pin, if it currently exists
	bool GetPinScreenData(Id pinId, PinType type, v2& outPosition, Color& outColor);
	// Returns the currently hovered pin, if any. Valid after EndNodeEditor()
	bool GetHoveredPin(Id& outPinId, PinType& outType);

	// Selection is queried and changed after EndNodeEditor()
	int NumSelectedLinks();
	const TArray<ast::Id>& GetSelectedNodes();
	bool GetSelectedLinks(TArray<ast::Id>& linkIds);
	void ClearNodeSelection();
	void ClearLinkSelection();
	void ClearNodeSelection(ast::Id nodeId);
	void ClearLinkSelection(Id linkId);
	void SelectNode(ast::Id nodeId);
	void SelectLink(Id linkId);
	bool IsNodeSelected(ast::Id nodeId);
	bool IsLinkSelected(Id linkId);
	bool IsLinkSelectedByIdx(i32 linkIdx);

	// True while the left mouse button is pressed over the content of a pin
	bool IsPinActive();
	// True while any pin is active. Sets the active pin id when given
	bool IsAnyPinActive(Id* attributeId = nullptr);

	// Link state changes, valid after EndNodeEditor()
	bool IsDraggingLink();
	TPair<Id, PinType> GetDraggedOriginPin();
	// True when a dragged link is dropped away from a pin. includingDetachedLinks also reports
	// links detached from a pin
	bool IsLinkDropped(
	    Id* outputId = nullptr, Id* inputId = nullptr, bool includingDetachedLinks = true);
	bool IsLinkCreated(Id& outputPinId, Id& inputPinId, bool* createdFromSnap = nullptr);
	bool IsLinkCreated(ast::Id& outputNodeId, Id& outputPinId, ast::Id& inputNodeId, Id& inputPinId,
	    bool* createdFromSnap = nullptr);

	// True when an existing link was detached. Returns its id in linkId
	bool IsLinkDestroyed(Id& linkId);
}    // namespace rift::Nodes
