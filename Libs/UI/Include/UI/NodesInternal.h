// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "UI/Nodes.h"
#include "UI/NodesMiniMap.h"
#include "UI/UIImGui.h"

#include <assert.h>
#include <Containers/Array.h>
#include <Containers/BitArray.h>
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

	typedef int UIState;
	typedef int ClickInteractionType;
	typedef int LinkCreationType;

	enum class Scope : u8
	{
		None   = 0,
		Editor = 1 << 0,
		Node   = 1 << 1,
		Pin    = 1 << 2
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
	//     i32 id;
	// };
	template<typename T>
	struct ObjectPool
	{
		TArray<T> Pool;
		BitArray InUse;
		TArray<i32> availableIds;
		ImGuiStorage IdMap;
	};


	// Emulates std::optional<i32> using the sentinel value `INVALID_INDEX`.
	struct OptionalIndex
	{
		OptionalIndex() : index(INVALID_INDEX) {}
		OptionalIndex(const i32 value) : index(value) {}
		OptionalIndex& operator=(const i32 value)
		{
			index = value;
			return *this;
		}

		i32 Value() const
		{
			assert(IsValid());
			return index;
		}

		void Reset()
		{
			index = INVALID_INDEX;
		}

		bool IsValid() const
		{
			return index != INVALID_INDEX;
		}

		bool operator==(const OptionalIndex& rhs) const
		{
			return index == rhs.index;
		}
		bool operator!=(const OptionalIndex& rhs) const
		{
			return index != rhs.index;
		}

		bool operator==(const i32 rhs) const
		{
			return index == rhs;
		}
		bool operator!=(const i32 rhs) const
		{
			return index != rhs;
		}


		static constexpr i32 INVALID_INDEX = -1;

	private:
		i32 index;
	};


	struct NodeData
	{
		Id id;
		v2 Origin = v2::Zero();    // The node origin is in editor space
		Rect TitleBarContentRect;
		Rect Rect{v2::Zero(), v2::Zero()};

		struct
		{
			Color Background, BackgroundHovered, BackgroundSelected, Outline, Titlebar,
			    TitlebarHovered, TitlebarSelected;
		} colorStyle;

		struct
		{
			float CornerRounding;
			v2 Padding;
			float BorderThickness;
		} LayoutStyle;

		ImVector<i32> inputs;
		ImVector<i32> outputs;
		bool Draggable = true;


		NodeData(const Id id = NoId()) : id(id) {}
	};


	struct PinData
	{
		Id id;
		i32 parentNodeIdx = NO_INDEX;
		Rect rect;
		PinShape Shape = PinShape_CircleFilled;
		v2 position;    // screen-space coordinates
		i32 Flags = PinFlags_None;

		struct
		{
			Color Background, Hovered;
		} colorStyle;

		PinData(const Id id = NoId()) : id(id) {}
	};

	struct LinkData
	{
		Id id        = NoId();
		Id outputPin = NoId();
		Id inputPin  = NoId();

		struct
		{
			Color Base, Hovered, Selected;
		} colorStyle;

		LinkData(const Id linkId = NoId()) : id(linkId) {}
	};

	struct ClickInteractionState
	{
		ClickInteractionType type;

		struct
		{
			i32 outputPin;
			i32 inputPin;
			LinkCreationType type;
		} linkCreation;

		struct
		{
			Rect Rect;    // Coordinates in grid space
		} boxSelector;

		ClickInteractionState() : type(ClickInteractionType_None) {}
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
		ObjectPool<PinData> outputs;
		ObjectPool<PinData> inputs;
		ObjectPool<LinkData> Links;

		ImVector<i32> NodeDepthOrder;

		// ui related fields
		v2 Panning = v2::Zero();
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

		ClickInteractionState clickInteraction;

		MiniMap miniMap;

		ObjectPool<PinData>& GetPinPool(PinType type)
		{
			switch (type)
			{
				default:
				case PinType::Output: return outputs;
				case PinType::Input: return inputs;
			}
		}
		const ObjectPool<PinData>& GetPinPool(PinType type) const
		{
			switch (type)
			{
				default:
				case PinType::Output: return outputs;
				case PinType::Input: return inputs;
			}
		}
		PinData& GetPinData(PinId pin)
		{
			// TODO: Incompatible id types!
			return GetPinPool(pin.type).Pool[pin.index];
		}
		const PinData& GetPinData(PinId pin) const
		{
			// TODO: Incompatible id types!
			return GetPinPool(pin.type).Pool[i32(pin.index)];
		}
	};

	struct Context
	{
		EditorContext* DefaultEditorCtx;
		EditorContext* EditorCtx;

		// Canvas draw list and helper state
		ImDrawList* CanvasDrawList;
		ImGuiStorage NodeIdxToSubmissionIdx;
		ImVector<i32> NodeIdxSubmissionOrder;
		ImVector<i32> NodeIndicesOverlappingWithMouse;
		ImVector<PinId> occludedPinIndices;

		// Canvas extents
		v2 CanvasOriginScreenSpace;
		Rect CanvasRectScreenSpace;

		// Debug helpers
		Scope currentScope;

		// Configuration state
		IO Io;
		Style Style;
		ImVector<ColElement> ColorModifierStack;
		ImVector<StyleVarElement> StyleModifierStack;
		ImGuiTextBuffer TextBuffer;

		i32 CurrentPinFlags;
		ImVector<i32> pinFlagStack;

		// UI element state
		i32 CurrentNodeIdx;
		PinId CurrentPinIdx;
		i32 CurrentPinId;

		OptionalIndex HoveredNodeIdx;
		OptionalIndex HoveredLinkIdx;
		PinId HoveredPinIdx;

		OptionalIndex DeletedLinkIdx;
		OptionalIndex SnapLinkIdx;

		// Event helper state
		// TODO: this should be a part of a state machine, and not a member of the global struct.
		// Unclear what parts of the code this relates to.
		i32 UIState;

		i32 activePinId;
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
	static inline i32 ObjectPoolFind(const ObjectPool<T>& objects, const Id id)
	{
		return objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);
	}

	template<typename T>
	static inline void ObjectPoolUpdate(ObjectPool<T>& objects)
	{
		for (i32 i = 0; i < objects.InUse.Size(); ++i)
		{
			const Id id = objects.Pool[i].id;

			if (!objects.InUse.IsSet(i) && objects.IdMap.GetInt(id, -1) == i)
			{
				objects.IdMap.SetInt(id, -1);
				objects.availableIds.Add(i);
				(objects.Pool.Data() + i)->~T();
			}
		}
	}

	template<>
	inline void ObjectPoolUpdate(ObjectPool<NodeData>& nodes)
	{
		for (i32 i = 0; i < nodes.InUse.Size(); ++i)
		{
			if (nodes.InUse.IsSet(i))
			{
				auto& node = nodes.Pool[i];
				node.inputs.clear();
				node.outputs.clear();
			}
			else
			{
				const Id id = nodes.Pool[i].id;

				if (nodes.IdMap.GetInt(i32(id), -1) == i)
				{
					// Remove node idx form depth stack the first time we detect that this idx slot
					// is unused
					ImVector<i32>& depth_stack = GetEditorContext().NodeDepthOrder;
					const i32* const elem      = depth_stack.find(i);
					assert(elem != depth_stack.end());
					depth_stack.erase(elem);

					nodes.IdMap.SetInt(i32(id), -1);
					nodes.availableIds.Add(i);
					(nodes.Pool.Data() + i)->~NodeData();
				}
			}
		}
	}

	template<typename T>
	static inline void ObjectPoolReset(ObjectPool<T>& objects)
	{
		if (!objects.InUse.IsEmpty())
		{
			objects.InUse.ClearBits();
		}
	}

	template<typename T>
	static inline i32 ObjectPoolFindOrCreateIndex(ObjectPool<T>& objects, const i32 id)
	{
		i32 index = objects.IdMap.GetInt(static_cast<ImGuiID>(id), -1);

		// Construct new object
		if (index == -1)
		{
			if (objects.availableIds.IsEmpty())
			{
				index = objects.Pool.Size();
				IM_ASSERT(objects.Pool.Size() == objects.InUse.Size());
				const i32 new_size = objects.Pool.Size() + 1;
				objects.Pool.Resize(new_size);
				objects.InUse.Resize(new_size);
			}
			else
			{
				index = objects.availableIds.Last();
				objects.availableIds.RemoveLast();
			}
			IM_PLACEMENT_NEW(objects.Pool.Data() + index) T(id);
			objects.IdMap.SetInt(static_cast<ImGuiID>(id), index);
		}

		// Flag it as used
		objects.InUse.FillBit(index);

		return index;
	}

	template<>
	inline i32 ObjectPoolFindOrCreateIndex(ObjectPool<NodeData>& nodes, const Id nodeId)
	{
		i32 nodeIdx = nodes.IdMap.GetInt(static_cast<ImGuiID>(i32(nodeId)), -1);

		// Construct new node
		if (nodeIdx == -1)
		{
			if (nodes.availableIds.IsEmpty())
			{
				nodeIdx = nodes.Pool.Size();
				IM_ASSERT(nodes.Pool.Size() == nodes.InUse.Size());
				const i32 new_size = nodes.Pool.Size() + 1;
				nodes.Pool.Resize(new_size);
				nodes.InUse.Resize(new_size);
			}
			else
			{
				nodeIdx = nodes.availableIds.Last();
				nodes.availableIds.RemoveLast();
			}
			IM_PLACEMENT_NEW(nodes.Pool.Data() + nodeIdx) NodeData(nodeId);
			nodes.IdMap.SetInt(static_cast<ImGuiID>(i32(nodeId)), nodeIdx);

			EditorContext& editor = GetEditorContext();
			editor.NodeDepthOrder.push_back(nodeIdx);
		}

		// Flag node as used
		nodes.InUse.FillBit(nodeIdx);

		return nodeIdx;
	}

	template<typename T>
	static inline T& ObjectPoolFindOrCreateObject(ObjectPool<T>& objects, const Id id)
	{
		const int index = ObjectPoolFindOrCreateIndex(objects, id);
		return objects.Pool[index];
	}
}    // namespace Rift::Nodes
