#pragma once

#include <Math/Color.h>
#include <Math/Vector.h>
#include <stddef.h>


#ifdef IMNODES_USER_CONFIG
#	include IMNODES_USER_CONFIG
#endif

struct ImGuiContext;


namespace Rift::Nodes
{
	typedef int StyleVar;           // -> enum StyleVar_
	typedef int StyleFlags;         // -> enum StyleFlags_
	typedef int PinShape;           // -> enum PinShape_
	typedef int AttributeFlags;     // -> enum AttributeFlags_
	typedef int MiniMapLocation;    // -> enum MiniMapLocation_

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
		StyleVar_LinkLineSegmentsPerLength,
		StyleVar_LinkHoverDistance,
		StyleVar_PinCircleRadius,
		StyleVar_PinQuadSideLength,
		StyleVar_PinTriangleSideLength,
		StyleVar_PinLineThickness,
		StyleVar_PinHoverRadius,
		StyleVar_PinOffset,
		StyleVar_MiniMapPadding,
		StyleVar_miniMapOffset,
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
		PinShape_QuadFilled
	};

	// This enum controls the way the attribute pins behave.
	enum AttributeFlags_
	{
		AttributeFlags_None = 0,
		// Allow detaching a link by left-clicking and dragging the link at a pin it is connected
		// to. NOTE: the user has to actually delete the link for this to work. A deleted link can
		// be detected by calling IsLinkDestroyed() after EndNodeEditor().
		AttributeFlags_EnableLinkDetachWithDragClick = 1 << 0,
		// Visual snapping of an in progress link will trigger IsLink Created/Destroyed events.
		// Allows for previewing the creation of a link while dragging it across attributes. NOTE:
		// the user has to actually delete the link for this to work. A deleted link can be detected
		// by calling IsLinkDestroyed() after EndNodeEditor().
		AttributeFlags_EnableLinkCreationOnSnap = 1 << 1
	};

	struct IO
	{
		struct EmulateThreeButtonMouse
		{
			EmulateThreeButtonMouse();

			// The keyboard modifier to use in combination with mouse left click to pan the editor
			// view. Set to NULL by default. To enable this feature, set the modifier to point to a
			// boolean indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().EmulateThreeButtonMouse.Modifier = &ImGui::GetIO().KeyAlt;
			const bool* Modifier;
		} EmulateThreeButtonMouse;

		struct LinkDetachWithModifierClick
		{
			LinkDetachWithModifierClick();

			// Pointer to a boolean value indicating when the desired modifier is pressed. Set to
			// NULL by default. To enable the feature, set the modifier to point to a boolean
			// indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
			//
			// Left-clicking a link with this modifier pressed will detach that link. NOTE: the user
			// has to actually delete the link for this to work. A deleted link can be detected by
			// calling IsLinkDestroyed() after EndNodeEditor().
			const bool* Modifier;
		} LinkDetachWithModifierClick;

		struct MultipleSelectModifier
		{
			MultipleSelectModifier();

			// Pointer to a boolean value indicating when the desired modifier is pressed. Set to
			// NULL by default. To enable the feature, set the modifier to point to a boolean
			// indicating the state of a modifier. For example,
			//
			// Nodes::GetIO().MultipleSelectModifier.Modifier = &ImGui::GetIO().KeyCtrl;
			//
			// Left-clicking a node with this modifier pressed will add the node to the list of
			// currently selected nodes. If this value is NULL, the Ctrl key will be used.
			const bool* Modifier;
		} MultipleSelectModifier;

		// Holding alt mouse button pans the node area, by default middle mouse button will be used
		// Set based on ImGuiMouseButton values
		int AltMouseButton;

		// Panning speed when dragging an element and mouse is outside the main editor view.
		float AutoPanningSpeed;


		IO();
	};

	struct Style
	{
		float GridSpacing;

		float NodeCornerRounding;
		v2 NodePadding;
		float NodeBorderThickness;

		float LinkThickness;
		float LinkLineSegmentsPerLength;
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


	enum MiniMapLocation_
	{
		MiniMapLocation_BottomLeft,
		MiniMapLocation_BottomRight,
		MiniMapLocation_TopLeft,
		MiniMapLocation_TopRight,
	};


	struct Context;

	// An editor context corresponds to a set of nodes in a single workspace (created with a single
	// Begin/EndNodeEditor pair)
	//
	// By default, the library creates an editor context behind the scenes, so using any of the
	// nodes functions don't require you to explicitly create a context.
	struct EditorContext;

// Callback type used to specify special behavior when hovering a node in the minimap
#ifndef MiniMapNodeHoveringCallback
	typedef void (*MiniMapNodeHoveringCallback)(int, void*);
#endif

#ifndef MiniMapNodeHoveringCallbackUserData
	typedef void* MiniMapNodeHoveringCallbackUserData;
#endif


	// Call this function if you are compiling UI in to a dll, separate from ImGui. Calling
	// this function sets the GImGui global variable, which is not shared across dll
	// boundaries.
	void SetImGuiContext(ImGuiContext* ctx);

	Context* CreateContext();
	void DestroyContext(Context* ctx = NULL);    // NULL = destroy current context
	Context* GetCurrentContext();
	void SetCurrentContext(Context* ctx);

	EditorContext* EditorContextCreate();
	void EditorContextFree(EditorContext*);
	void EditorContextSet(EditorContext*);
	v2 GetEditorContextPanning();
	void EditorContextResetPanning(const v2& pos);
	void EditorContextMoveToNode(const int node_id);

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
	v2 ScreenToGridPosition(const v2& v);
	v2 GridToScreenPosition(const v2& v);
	v2 GridToEditorPosition(const v2& v);
	v2 EditorToGridPosition(const v2& v);
	v2 MiniMapToGridPosition(const v2& v);
	v2 ScreenToMiniMapPosition(const v2& v);


	// Add a navigable minimap to the editor; call before EndNodeEditor after all
	// nodes and links have been specified
	void MiniMap(const float minimap_size_fraction               = 0.2f,
	    const MiniMapLocation location                           = MiniMapLocation_TopLeft,
	    const MiniMapNodeHoveringCallback node_hovering_callback = NULL,
	    const MiniMapNodeHoveringCallbackUserData node_hovering_callback_data = NULL);

	// Use PushStyleColor and PopStyleColor to modify Style::Colors mid-frame.
	void PushStyleColor(ColorVar item, Color color);
	void PopStyleColor(i32 count = 1);
	void PushStyleVar(StyleVar style_item, float value);
	void PushStyleVar(StyleVar style_item, const v2& value);
	void PopStyleVar(int count = 1);

	// id can be any positive or negative integer, but INT_MIN is currently reserved for
	// internal use.
	void BeginNode(int id);
	void EndNode();

	v2 GetNodeDimensions(int id);

	// Place your node title bar content (such as the node title, using ImGui::Text) between the
	// following function calls. These functions have to be called before adding any attributes,
	// or the layout of the node will be incorrect.
	void BeginNodeTitleBar();
	void EndNodeTitleBar();

	// Attributes are ImGui UI elements embedded within the node. Attributes can have pin shapes
	// rendered next to them. Links are created between pins.
	//
	// The activity status of an attribute can be checked via the IsAttributeActive() and
	// IsAnyAttributeActive() function calls. This is one easy way of checking for any changes
	// made to an attribute's drag float UI, for instance.
	//
	// Each attribute id must be unique.

	// Create an input attribute block. The pin is rendered on left side.
	void BeginInputAttribute(int id, PinShape shape = PinShape_CircleFilled);
	void EndInputAttribute();
	// Create an output attribute block. The pin is rendered on the right side.
	void BeginOutputAttribute(int id, PinShape shape = PinShape_CircleFilled);
	void EndOutputAttribute();
	// Create a static attribute block. A static attribute has no pin, and therefore can't be
	// linked to anything. However, you can still use IsAttributeActive() and
	// IsAnyAttributeActive() to check for attribute activity.
	void BeginStaticAttribute(int id);
	void EndStaticAttribute();

	// Push a single AttributeFlags value. By default, only AttributeFlags_None is set.
	void PushAttributeFlag(AttributeFlags flag);
	void PopAttributeFlag();

	// Render a link between attributes.
	// The attributes ids used here must match the ids used in Begin(Input|Output)Attribute
	// function calls. The order of start_attr and end_attr doesn't make a difference for
	// rendering the link.
	void Link(int id, int start_attribute_id, int end_attribute_id);

	// Enable or disable the ability to click and drag a specific node.
	void SetNodeDraggable(int node_id, const bool draggable);

	// The node's position can be expressed in three coordinate systems:
	// * screen space coordinates, -- the origin is the upper left corner of the window.
	// * editor space coordinates -- the origin is the upper left corner of the node editor
	// window
	// * grid space coordinates, -- the origin is the upper left corner of the node editor
	// window, translated by the current editor panning vector (see GetEditorContextPanning()
	// and EditorContextResetPanning())

	// Use the following functions to get and set the node's coordinates in these coordinate
	// systems.

	void SetNodeScreenSpacePos(int node_id, const v2& screen_space_pos);
	void SetNodeEditorSpacePos(int node_id, const v2& editor_space_pos);
	void SetNodeGridSpacePos(int node_id, const v2& grid_pos);

	v2 GetNodeScreenSpacePos(const int node_id);
	v2 GetNodeEditorSpacePos(const int node_id);
	v2 GetNodeGridSpacePos(const int node_id);

	// Returns true if the current node editor canvas is being hovered over by the mouse, and is
	// not blocked by any other windows.
	bool IsEditorHovered();
	// The following functions return true if a UI element is being hovered over by the mouse
	// cursor. Assigns the id of the UI element being hovered over to the function argument. Use
	// these functions after EndNodeEditor() has been called.
	bool IsNodeHovered(int* node_id);
	bool IsLinkHovered(int* link_id);
	bool IsPinHovered(int* attribute_id);

	// Use The following two functions to query the number of selected nodes or links in the
	// current editor. Use after calling EndNodeEditor().
	int NumSelectedNodes();
	int NumSelectedLinks();
	// Get the selected node/link ids. The pointer argument should point to an integer array
	// with at least as many elements as the respective NumSelectedNodes/NumSelectedLinks
	// function call returned.
	void GetSelectedNodes(int* node_ids);
	void GetSelectedLinks(int* link_ids);
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
	void SelectNode(int node_id);
	void ClearNodeSelection(int node_id);
	bool IsNodeSelected(int node_id);
	void SelectLink(int link_id);
	void ClearLinkSelection(int link_id);
	bool IsLinkSelected(int link_id);

	// Was the previous attribute active? This will continuously return true while the left
	// mouse button is being pressed over the UI content of the attribute.
	bool IsAttributeActive();
	// Was any attribute active? If so, sets the active attribute id to the output function
	// argument.
	bool IsAnyAttributeActive(int* attribute_id = NULL);

	// Use the following functions to query a change of state for an existing link, or new link.
	// Call these after EndNodeEditor().

	// Did the user start dragging a new link from a pin?
	bool IsLinkStarted(int* started_at_attribute_id);
	// Did the user drop the dragged link before attaching it to a pin?
	// There are two different kinds of situations to consider when handling this event:
	// 1) a link which is created at a pin and then dropped
	// 2) an existing link which is detached from a pin and then dropped
	// Use the including_detached_links flag to control whether this function triggers when the
	// user detaches a link and drops it.
	bool IsLinkDropped(int* started_at_attribute_id = NULL, bool including_detached_links = true);
	// Did the user finish creating a new link?
	bool IsLinkCreated(
	    int* started_at_attribute_id, int* ended_at_attribute_id, bool* created_from_snap = NULL);
	bool IsLinkCreated(int* started_at_node_id, int* started_at_attribute_id, int* ended_at_node_id,
	    int* ended_at_attribute_id, bool* created_from_snap = NULL);

	// Was an existing link detached from a pin by the user? The detached link's id is assigned
	// to the output argument link_id.
	bool IsLinkDestroyed(int* link_id);
}    // namespace Rift::Nodes
