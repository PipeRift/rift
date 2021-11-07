// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "UI/Nodes.h"
#include "UI/NodesMiniMap.h"
#include "UI/UIImGui.h"

#include <assert.h>
#include <limits.h>
#include <Math/Vector.h>


// the structure of this file:
//
// [SECTION] internal enums
// [SECTION] internal data structures
// [SECTION] global and editor context structs
// [SECTION] object pool implementation

namespace Rift::Nodes
{
	struct Context;

	extern Context* gNodes;

	// [SECTION] internal enums

	typedef int Scope;
	typedef int UIState;
	typedef int ClickInteractionType;
	typedef int LinkCreationType;

	enum Scope_
	{
		Scope_None   = 1,
		Scope_Editor = 1 << 1,
		Scope_Node   = 1 << 2,
		Scope_Pin    = 1 << 3
	};

	enum UIState_
	{
		UIState_None        = 0,
		UIState_LinkStarted = 1 << 0,
		UIState_LinkDropped = 1 << 1,
		UIState_LinkCreated = 1 << 2
	};

	enum ClickInteractionType_
	{
		ClickInteractionType_Node,
		ClickInteractionType_Link,
		ClickInteractionType_LinkCreation,
		ClickInteractionType_Panning,
		ClickInteractionType_BoxSelection,
		ClickInteractionType_ImGuiItem,
		ClickInteractionType_None
	};

	enum LinkCreationType_
	{
		LinkCreationType_Standard,
		LinkCreationType_FromDetach
	};

	// [SECTION] internal data structures

	// The object T must have the following interface:
	//
	// struct T
	// {
	//     T();
	//
	//     int id;
	// };
	template<typename T>
	struct ObjectPool
	{
		ImVector<T> Pool;
		ImVector<bool> InUse;
		ImVector<int> FreeList;
		ImGuiStorage IdMap;

		ObjectPool() : Pool(), InUse(), FreeList(), IdMap() {}
	};

	// Emulates std::optional<int> using the sentinel value `INVALID_INDEX`.
	struct ImOptionalIndex
	{
		ImOptionalIndex() : _Index(INVALID_INDEX) {}
		ImOptionalIndex(const int value) : _Index(value) {}

		// Observers

		inline bool HasValue() const
		{
			return _Index != INVALID_INDEX;
		}

		inline int Value() const
		{
			assert(HasValue());
			return _Index;
		}

		// Modifiers

		inline ImOptionalIndex& operator=(const int value)
		{
			_Index = value;
			return *this;
		}

		inline void Reset()
		{
			_Index = INVALID_INDEX;
		}

		inline bool operator==(const ImOptionalIndex& rhs) const
		{
			return _Index == rhs._Index;
		}

		inline bool operator==(const int rhs) const
		{
			return _Index == rhs;
		}

		inline bool operator!=(const ImOptionalIndex& rhs) const
		{
			return _Index != rhs._Index;
		}

		inline bool operator!=(const int rhs) const
		{
			return _Index != rhs;
		}

		static const int INVALID_INDEX = -1;

	private:
		int _Index;
	};

	struct NodeData
	{
		i32 Id;
		v2 Origin;    // The node origin is in editor space
		Rect TitleBarContentRect;
		Rect Rect;

		struct
		{
			Color Background, BackgroundHovered, BackgroundSelected, Outline, Titlebar,
			    TitlebarHovered, TitlebarSelected;
		} ColorStyle;

		struct
		{
			float CornerRounding;
			v2 Padding;
			float BorderThickness;
		} LayoutStyle;

		ImVector<int> pinIndices;
		bool Draggable;

		NodeData(const int node_id)
		    : Id(node_id)
		    , Origin(0.0f, 0.0f)
		    , TitleBarContentRect()
		    , Rect(v2::Zero(), v2::Zero())
		    , ColorStyle()
		    , LayoutStyle()
		    , pinIndices()
		    , Draggable(true)
		{}

		~NodeData()
		{
			Id = INT_MIN;
		}
	};

	struct PinData
	{
		i32 Id;
		i32 ParentNodeIdx;
		Rect rect;
		PinType Type;
		PinShape Shape;
		v2 Pos;    // screen-space coordinates
		i32 Flags;

		struct
		{
			Color Background, Hovered;
		} ColorStyle;

		PinData(const i32 pin_id)
		    : Id(pin_id)
		    , ParentNodeIdx()
		    , rect()
		    , Type(PinType::None)
		    , Shape(PinShape_CircleFilled)
		    , Pos()
		    , Flags(PinFlags_None)
		    , ColorStyle()
		{}
	};

	struct LinkData
	{
		i32 Id;
		i32 StartPinIdx, EndPinIdx;

		struct
		{
			Color Base, Hovered, Selected;
		} ColorStyle;

		LinkData(const i32 link_id) : Id(link_id), StartPinIdx(), EndPinIdx(), ColorStyle() {}
	};

	struct ImClickInteractionState
	{
		ClickInteractionType Type;

		struct
		{
			i32 StartPinIdx;
			ImOptionalIndex EndPinIdx;
			LinkCreationType Type;
		} LinkCreation;

		struct
		{
			Rect Rect;    // Coordinates in grid space
		} BoxSelector;

		ImClickInteractionState() : Type(ClickInteractionType_None) {}
	};

	struct ColElement
	{
		ColorVar item;
		Color color;

		ColElement(ColorVar item, Color color) : item(item), color(color) {}
	};

	struct StyleVarElement
	{
		StyleVar item;
		float value[2];

		StyleVarElement(StyleVar item, float scalar) : item(item)
		{
			value[0] = scalar;
		}

		StyleVarElement(StyleVar item, v2 value) : item(item)
		{
			value[0] = value.x;
			value[1] = value.y;
		}
	};

	// [SECTION] global and editor context structs


	struct EditorContext
	{
		ObjectPool<NodeData> nodes;
		ObjectPool<PinData> pins;
		ObjectPool<LinkData> Links;

		ImVector<i32> NodeDepthOrder;

		// ui related fields
		v2 Panning;
		v2 AutoPanningDelta;
		// Minimum and maximum extents of all content in grid space. Valid after final
		// Nodes::EndNode() call.
		Rect gridContentBounds;

		ImVector<i32> SelectedNodeIndices;
		ImVector<i32> SelectedLinkIndices;

		// Relative origins of selected nodes for snapping of dragged nodes
		ImVector<v2> SelectedNodeOrigins;
		// Offset of the primary node origin relative to the mouse cursor.
		v2 PrimaryNodeOffset;

		ImClickInteractionState clickInteraction;

		MiniMap miniMap;


		EditorContext()
		    : nodes()
		    , pins()
		    , Links()
		    , Panning(0.f, 0.f)
		    , SelectedNodeIndices()
		    , SelectedLinkIndices()
		    , clickInteraction()
		{}
	};

	struct Context
	{
		EditorContext* DefaultEditorCtx;
		EditorContext* EditorCtx;

		// Canvas draw list and helper state
		ImDrawList* CanvasDrawList;
		ImGuiStorage NodeIdxToSubmissionIdx;
		ImVector<int> NodeIdxSubmissionOrder;
		ImVector<int> NodeIndicesOverlappingWithMouse;
		ImVector<int> occludedPinIndices;

		// Canvas extents
		v2 CanvasOriginScreenSpace;
		Rect CanvasRectScreenSpace;

		// Debug helpers
		Scope CurrentScope;

		// Configuration state
		IO Io;
		Style Style;
		ImVector<ColElement> ColorModifierStack;
		ImVector<StyleVarElement> StyleModifierStack;
		ImGuiTextBuffer TextBuffer;

		int CurrentPinFlags;
		ImVector<int> pinFlagStack;

		// UI element state
		int CurrentNodeIdx;
		int CurrentPinIdx;
		int CurrentPinId;

		ImOptionalIndex HoveredNodeIdx;
		ImOptionalIndex HoveredLinkIdx;
		ImOptionalIndex HoveredPinIdx;

		ImOptionalIndex DeletedLinkIdx;
		ImOptionalIndex SnapLinkIdx;

		// Event helper state
		// TODO: this should be a part of a state machine, and not a member of the global struct.
		// Unclear what parts of the code this relates to.
		int UIState;

		int activePinId;
		bool activePin;

		// ImGui::IO cache

		v2 mousePosition;

		bool LeftMouseClicked;
		bool leftMouseReleased;
		bool AltMouseClicked;
		bool leftMouseDragging;
		bool AltMouseDragging;
		float AltMouseScrollDelta;
		bool multipleSelectModifier;
	};


	// [SECTION] ObjectPool implementation

	template<typename T>
	static inline int ObjectPoolFind(const ObjectPool<T>& objects, const int id)
	{
		const int index = objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);
		return index;
	}

	template<typename T>
	static inline void ObjectPoolUpdate(ObjectPool<T>& objects)
	{
		for (int i = 0; i < objects.InUse.size(); ++i)
		{
			const int id = objects.Pool[i].Id;

			if (!objects.InUse[i] && objects.IdMap.GetInt(id, -1) == i)
			{
				objects.IdMap.SetInt(id, -1);
				objects.FreeList.push_back(i);
				(objects.Pool.Data + i)->~T();
			}
		}
	}

	template<>
	inline void ObjectPoolUpdate(ObjectPool<NodeData>& nodes)
	{
		for (int i = 0; i < nodes.InUse.size(); ++i)
		{
			if (nodes.InUse[i])
			{
				nodes.Pool[i].pinIndices.clear();
			}
			else
			{
				const int id = nodes.Pool[i].Id;

				if (nodes.IdMap.GetInt(id, -1) == i)
				{
					// Remove node idx form depth stack the first time we detect that this idx slot
					// is unused
					ImVector<int>& depth_stack = GetEditorContext().NodeDepthOrder;
					const int* const elem      = depth_stack.find(i);
					assert(elem != depth_stack.end());
					depth_stack.erase(elem);

					nodes.IdMap.SetInt(id, -1);
					nodes.FreeList.push_back(i);
					(nodes.Pool.Data + i)->~NodeData();
				}
			}
		}
	}

	template<typename T>
	static inline void ObjectPoolReset(ObjectPool<T>& objects)
	{
		if (!objects.InUse.empty())
		{
			memset(objects.InUse.Data, 0, objects.InUse.size_in_bytes());
		}
	}

	template<typename T>
	static inline int ObjectPoolFindOrCreateIndex(ObjectPool<T>& objects, const int id)
	{
		int index = objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);

		// Construct new object
		if (index == -1)
		{
			if (objects.FreeList.empty())
			{
				index = objects.Pool.size();
				IM_ASSERT(objects.Pool.size() == objects.InUse.size());
				const int new_size = objects.Pool.size() + 1;
				objects.Pool.resize(new_size);
				objects.InUse.resize(new_size);
			}
			else
			{
				index = objects.FreeList.back();
				objects.FreeList.pop_back();
			}
			IM_PLACEMENT_NEW(objects.Pool.Data + index) T(id);
			objects.IdMap.SetInt(static_cast<ImGuiID>(id), index);
		}

		// Flag it as used
		objects.InUse[index] = true;

		return index;
	}

	template<>
	inline int ObjectPoolFindOrCreateIndex(ObjectPool<NodeData>& nodes, const int node_id)
	{
		int node_idx = nodes.IdMap.GetInt(static_cast<ImGuiID>(node_id), -1);

		// Construct new node
		if (node_idx == -1)
		{
			if (nodes.FreeList.empty())
			{
				node_idx = nodes.Pool.size();
				IM_ASSERT(nodes.Pool.size() == nodes.InUse.size());
				const int new_size = nodes.Pool.size() + 1;
				nodes.Pool.resize(new_size);
				nodes.InUse.resize(new_size);
			}
			else
			{
				node_idx = nodes.FreeList.back();
				nodes.FreeList.pop_back();
			}
			IM_PLACEMENT_NEW(nodes.Pool.Data + node_idx) NodeData(node_id);
			nodes.IdMap.SetInt(static_cast<ImGuiID>(node_id), node_idx);

			EditorContext& editor = GetEditorContext();
			editor.NodeDepthOrder.push_back(node_idx);
		}

		// Flag node as used
		nodes.InUse[node_idx] = true;

		return node_idx;
	}

	template<typename T>
	static inline T& ObjectPoolFindOrCreateObject(ObjectPool<T>& objects, const int id)
	{
		const int index = ObjectPoolFindOrCreateIndex(objects, id);
		return objects.Pool[index];
	}
}    // namespace Rift::Nodes
