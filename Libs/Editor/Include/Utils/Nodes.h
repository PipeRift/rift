// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <Pipe/Core/EnumFlags.h>
#include <Pipe/Math/Color.h>
#include <Pipe/Math/Vector.h>
#include <UI/UIImgui.h>


#ifdef IMNODES_USER_CONFIG
#	include IMNODES_USER_CONFIG
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


	// This enum controls the way the attribute pins behave.
	enum PinFlags_
	{
		PinFlags_None = 0,
		// Allow detaching a link by left-clicking and dragging the link at a pin it is connected
		// to. NOTE: the user has to actually delete the link for this to work. A deleted link can
		// be detected by calling IsLinkDestroyed() after EndNodeEditor().
		PinFlags_EnableLinkDetachWithDragClick = 1 << 0,
		// Visual snapping of an in progress link will trigger IsLink Created/Destroyed events.
		// Allows for previewing the creation of a link while dragging it across attributes. NOTE:
		// the user has to actually delete the link for this to work. A deleted link can be detected
		// by calling IsLinkDestroyed() after EndNodeEditor().
		PinFlags_EnableLinkCreationOnSnap = 1 << 1
	};

	struct IO
	{
		struct EmulateThreeButtonMouse
		{
			// The keyboard modifier to use in combination with mouse left click to pan the editor
			// view. Set to nullptr by default. To enable this feature, set the modifier to point to
			// a boolean indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().emulateThreeButtonMouse.modifier = &ImGui::GetIO().KeyAlt;
			const bool* modifier = nullptr;
		} emulateThreeButtonMouse;

		struct LinkDetachWithModifierClick
		{
			// Pointer to a boolean value indicating when the desired modifier is pressed. Set to
			// nullptr by default. To enable the feature, set the modifier to point to a boolean
			// indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().linkDetachWithModifierClick.modifier = &ImGui::GetIO().KeyCtrl;
			//
			// Left-clicking a link with this modifier pressed will detach that link. NOTE: the user
			// has to actually delete the link for this to work. A deleted link can be detected by
			// calling IsLinkDestroyed() after EndNodeEditor().
			const bool* modifier = nullptr;
		} linkDetachWithModifierClick;

		struct MultipleSelectModifier
		{
			// Pointer to a boolean value indicating when the desired modifier is pressed. Set to
			// nullptr by default. To enable the feature, set the modifier to point to a boolean
			// indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().multipleSelectModifier.modifier = &ImGui::GetIO().KeyCtrl;
			//
			// Left-clicking a node with this modifier pressed will add the node to the list of
			// currently selected nodes. If this value is nullptr, the Ctrl key will be used.
			const bool* modifier = nullptr;
		} multipleSelectModifier;

		// Holding alt mouse button pans the node area, by default middle mouse button will be used
		// Set based on ImGuiMouseButton values
		i32 AltMouseButton = ImGuiMouseButton_Middle;

		// Panning speed when dragging an element and mouse is outside the main editor view.
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

		// The following variables control the look and behavior of the pins. The default size of
		// each pin shape is balanced to occupy approximately the same surface area on the screen.

		// The circle radius used when the pin shape is either PinShape_Circle or
		// PinShape_CircleFilled.
		float PinCircleRadius;
		// The quad side length used when the shape is either PinShape_Quad or
		// PinShape_QuadFilled.
		float PinQuadSideLength;
		// The equilateral triangle side length used when the pin shape is either
		// PinShape_Triangle or PinShape_TriangleFilled.
		float PinTriangleSideLength;
		// The quad side length used when the shape is either PinShape_Diamond or
		// PinShape_DiamondFilled.
		float PinDiamondSideLength;
		// The thickness of the line used when the pin shape is not filled.
		float PinLineThickness;
		// The radius from the pin's center position inside of which it is detected as being hovered
		// over.
		float PinHoverRadius;
		// Offsets the pins' positions from the edge of the node to the outside of the node.
		float PinOffset;

		// Mini-map padding size between mini-map edge and mini-map content.
		v2 miniMapPadding{8.f, 8.f};
		// Mini-map offset from the screen side.
		v2 miniMapOffset{4.f, 4.f};

		// By default, StyleFlags_NodeOutline and StyleFlags_Gridlines are enabled.
		StyleFlags Flags;
		// Set these mid-frame using Push/PopStyleColor. You can index this color array with with a
		// Color value.
		Color colors[u8(ColorVar_COUNT)];

		Style();
	};

	struct CubicBezier
	{
		v2 p0, p1, p2, p3;
		i32 numSegments;
	};

	struct Context;

	// An editor context corresponds to a set of nodes in a single workspace (created with a single
	// Begin/EndNodeEditor pair)
	//
	// By default, the library creates an editor context behind the scenes, so using any of the
	// nodes functions don't require you to explicitly create a context.
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
	void MoveToNode(AST::Id nodeId, v2 offset = v2::Zero());

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


	// Use PushStyleColor and PopStyleColor to modify UI::Colors mid-frame.
	void PushStyleColor(ColorVar item, Color color);
	void PopStyleColor(i32 count = 1);
	void PushStyleVar(StyleVar style_item, float value);
	void PushStyleVar(StyleVar style_item, const v2& value);
	void PopStyleVar(i32 count = 1);

	// id can be any positive or negative integer, but INT_MIN is currently reserved for
	// internal use.
	void BeginNode(AST::Id id);
	void EndNode();

	v2 GetNodeDimensions(AST::Id id);

	// Place your node title bar content (such as the node title, using ImGui::Text) between the
	// following function calls. These functions have to be called before adding any attributes,
	// or the layout of the node will be incorrect.
	void BeginNodeTitleBar();
	void EndNodeTitleBar();

	// Pins are ImGui UI elements embedded within the node. Pins can have pin shapes
	// rendered next to them. Links are created between pins.
	//
	// The activity status of an attribute can be checked via the IsPinActive() and
	// IsAnyPinActive() function calls. This is one easy way of checking for any changes
	// made to an attribute's drag float UI, for instance.
	//
	// Each attribute id must be unique.

	// Create an input attribute block. The pin is rendered on left side.
	void BeginInput(Id id, PinShape shape = PinShape_CircleFilled);
	void EndInput();
	// Create an output attribute block. The pin is rendered on the right side.
	void BeginOutput(Id id, PinShape shape = PinShape_CircleFilled);
	void EndOutput();

	// Push a single PinFlags value. By default, only PinFlags_None is set.
	void PushPinFlag(PinFlags flag);
	void PopPinFlag();

	// Render a link between attributes.
	// The attributes ids used here must match the ids used in Begin(Input|Output)
	// function calls. The order of start_attr and end_attr doesn't make a difference for
	// rendering the link.
	void Link(Id id, Id outputPin, Id inputPin);

	// Enable or disable the ability to click and drag a specific node.
	void SetNodeDraggable(Id nodeId, const bool draggable);

	// The node's position can be expressed in three coordinate systems:
	// * screen space coordinates, -- the origin is the upper left corner of the window.
	// * editor space coordinates -- the origin is the upper left corner of the node editor
	// window
	// * grid space coordinates, -- the origin is the upper left corner of the node editor
	// window, translated by the current editor panning vector (see GetEditorContextPanning()
	// and EditorContextResetPanning())

	// Use the following functions to get and set the node's coordinates in these coordinate
	// systems.

	void SetNodeScreenSpacePos(AST::Id nodeId, const v2& screen_space_pos);
	void SetNodeEditorSpacePos(AST::Id nodeId, const v2& editor_space_pos);
	void SetNodeGridSpacePos(AST::Id nodeId, const v2& grid_pos);

	v2 GetNodeScreenSpacePos(AST::Id nodeId);
	v2 GetNodeEditorSpacePos(AST::Id nodeId);
	v2 GetNodeGridSpacePos(AST::Id nodeId);

	// Returns true if the current node editor canvas is being hovered over by the mouse, and is
	// not blocked by any other windows.
	bool IsEditorHovered();
	AST::Id GetHoveredNode();
	AST::Id GetHoveredLink();

	bool IsNodeHovered(AST::Id nodeId);
	bool IsLinkHovered(AST::Id linkId);
	bool IsPinHovered(Id* attributeId);

	// Use The following two functions to query the number of selected nodes or links in the
	// current editor. Use after calling EndNodeEditor().
	int NumSelectedLinks();
	// Get the selected node/link ids. The pointer argument should point to an integer array
	// with at least as many elements as the respective NumSelectedNodes/NumSelectedLinks
	// function call returned.
	const TArray<AST::Id>& GetSelectedNodes();
	bool GetSelectedLinks(TArray<AST::Id>& linkIds);
	// Clears the list of selected nodes/links. Useful if you want to delete a selected node or
	// link.
	void ClearNodeSelection();
	void ClearLinkSelection();
	// Use the following functions to add or remove individual nodes or links from the current
	// editors selection. Note that all functions require the id to be an existing valid id for
	// this editor. Select-functions has the precondition that the object is currently
	// considered unselected. Clear-functions has the precondition that the object is currently
	// considered selected. Preconditions listed above can be checked via
	// IsNodeSelected/IsLinkSelected if not already known.
	void ClearNodeSelection(AST::Id nodeId);
	void ClearLinkSelection(Id linkId);
	void SelectNode(AST::Id nodeId);
	void SelectLink(Id linkId);
	bool IsNodeSelected(AST::Id nodeId);
	bool IsLinkSelected(Id linkId);
	bool IsLinkSelectedByIdx(i32 linkIdx);

	// Was the previous pin active? This will continuously return true while the left
	// mouse button is being pressed over the UI content of the pin.
	bool IsPinActive();
	// Was any pin active? If so, sets the active pin id to the output function
	// argument.
	bool IsAnyPinActive(Id* attributeId = nullptr);

	// Use the following functions to query a change of state for an existing link, or new link.
	// Call these after EndNodeEditor().

	bool IsDraggingLink();
	TPair<Id, PinType> GetDraggedOriginPin();
	// Did the user drop the dragged link before attaching it to a pin?
	// There are two different kinds of situations to consider when handling this event:
	// 1) a link which is created at a pin and then dropped
	// 2) an existing link which is detached from a pin and then dropped
	// Use the including_detached_links flag to control whether this function triggers when the
	// user detaches a link and drops it.
	bool IsLinkDropped(
	    Id* outputId = nullptr, Id* inputId = nullptr, bool includingDetachedLinks = true);
	// Did the user finish creating a new link?
	bool IsLinkCreated(Id& outputPinId, Id& inputPinId, bool* createdFromSnap = nullptr);
	bool IsLinkCreated(AST::Id& outputNodeId, Id& outputPinId, AST::Id& inputNodeId, Id& inputPinId,
	    bool* createdFromSnap = nullptr);

	// Was an existing link detached from a pin by the user? The detached link's id is assigned
	// to the output argument linkId.
	bool IsLinkDestroyed(Id& linkId);
}    // namespace rift::Nodes
