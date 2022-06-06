// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "UI/UIImgui.h"
#include "Utils/Nodes.h"
#include "Utils/NodesMiniMap.h"

#include <assert.h>
#include <AST/Id.h>
#include <limits.h>
#include <PCore/Array.h>
#include <PCore/BitArray.h>
#include <PCore/EnumFlags.h>
#include <PMath/Vector.h>


// the structure of this file:
//
// [SECTION] internal enums
// [SECTION] internal data structures
// [SECTION] global and editor context structs
// [SECTION] object pool implementation

namespace rift::Nodes
{
	// using namespace p::EnumOperators;

	struct Context;

	extern Context* gNodes;

	// [SECTION] internal enums

	using UIState              = i32;
	using ClickInteractionType = i32;
	using LinkCreationType     = i32;

	enum class Scope : u8
	{
		None   = 0,
		Editor = 1 << 0,
		Node   = 1 << 1,
		Pin    = 1 << 2
	};
	DEFINE_FLAG_OPERATORS(Scope)

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
		ImGuiStorage idMap;
	};

	template<typename T>
	struct TIndexedArray
	{
		using Iterator      = TArray<AST::Id>::Iterator;
		using ConstIterator = TArray<AST::Id>::ConstIterator;


		TArray<AST::Id> frameIds;
		TArray<T> data;
		TArray<AST::Id> lastFrameIds;
		TArray<AST::Id> invalidIds;


	public:
		T& GetOrAdd(AST::Id id, bool* outAdded = nullptr)
		{
			const u32 index  = ecs::GetIndex(id);
			const bool added = frameIds.FindOrAddSorted(id).second;
			if (added && !lastFrameIds.ContainsSorted(id))
			{
				// Id added
				if (outAdded)
				{
					*outAdded = true;
				}
				if (index >= data.Size())
				{
					data.Resize(index + 1);
				}
				T* const ptr = GetByIndex(index);
				*ptr         = {};
				return *ptr;
			}
			return *GetByIndex(index);
		}

		T& Get(AST::Id id)
		{
			return *GetByIndex(ecs::GetIndex(id));
		}

		const T& Get(AST::Id id) const
		{
			return const_cast<TIndexedArray<T>*>(this)->Get(id);
		}

		T* TryGet(AST::Id id)
		{
			if (Contains(id))
			{
				return GetByIndex(ecs::GetIndex(id));
			}
			return nullptr;
		}

		bool Contains(AST::Id id) const
		{
			return frameIds.ContainsSorted(id);
		}

		T& operator[](AST::Id id)
		{
			return Get(id);
		}
		const T& operator[](AST::Id id) const
		{
			return Get(id);
		}

		void CacheInvalidIds()
		{
			invalidIds.Empty(false);
			for (AST::Id id : lastFrameIds)
			{
				if (!frameIds.ContainsSorted(id))
				{
					invalidIds.Add(id);
				}
			}
		}

		// Clears ids that have not been used for one frame from the container
		// @returns the invalid ids
		void SwapFrameIds()
		{
			lastFrameIds = Move(frameIds);
		}

		ConstIterator begin() const
		{
			return frameIds.begin();
		}

		ConstIterator end() const
		{
			return frameIds.end();
		}

	private:
		T* GetByIndex(u32 index)
		{
			Check(data.IsValidIndex(index));
			return data.Data() + index;
		}
	};

	template<typename T>
	struct NodeArray : public TIndexedArray<T>
	{
		TArray<AST::Id> depthOrder;


		T& GetOrAdd(AST::Id id, bool* outAdded = nullptr)
		{
			bool added = false;
			T& node    = TIndexedArray<T>::GetOrAdd(id, &added);
			// If id has not been added before
			if (added)
			{
				depthOrder.Add(id);
				if (outAdded)
				{
					*outAdded = true;
				}
			}
			return node;
		}

		void PushToTheFront(AST::Id id)
		{
			depthOrder.Remove(id, false);
			depthOrder.Add(id);
		}

		void ClearDepthOrder()
		{
			depthOrder.ExcludeIf([this](AST::Id id) {
				return TIndexedArray<T>::invalidIds.ContainsSorted(id);
			});
		}
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
		v2 Origin = v2::Zero();    // The node origin is in editor space
		Rect TitleBarContentRect;
		Rect rect{v2::Zero(), v2::Zero()};

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


		NodeData() {}
	};


	struct PinData
	{
		Id id;
		AST::Id parentNodeId = AST::NoId;
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
		Id outputIdx = NO_INDEX;
		Id inputIdx  = NO_INDEX;

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
			i32 outputIdx;
			i32 inputIdx;
			LinkCreationType type;
		} linkCreation;

		struct
		{
			Rect rect;    // Coordinates in grid space
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
		NodeArray<NodeData> nodes;
		ObjectPool<PinData> outputs;
		ObjectPool<PinData> inputs;
		ObjectPool<LinkData> Links;

		// ui related fields
		v2 panning = v2::Zero();
		v2 AutoPanningDelta;
		// Minimum and maximum extents of all content in grid space. Valid after final
		// Nodes::EndNode() call.
		Rect gridContentBounds;

		TArray<AST::Id> selectedNodeIds;
		ImVector<i32> selectedLinkIndices;

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
		PinData& GetPinData(PinIdx pin)
		{
			// TODO: Incompatible id types!
			return GetPinPool(pin.type).Pool[pin.index];
		}
		const PinData& GetPinData(PinIdx pin) const
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
		ImVector<AST::Id> nodeSubmissionOrder;
		ImVector<AST::Id> nodeIdsOverlappingWithMouse;
		ImVector<PinIdx> occludedPinIndices;

		// Canvas extents
		v2 CanvasOriginScreenSpace;
		Rect CanvasRectScreenSpace;

		// Debug helpers
		Scope currentScope;

		// Configuration state
		IO io;
		Style style;
		ImVector<ColElement> ColorModifierStack;
		ImVector<StyleVarElement> styleModifierStack;
		ImGuiTextBuffer TextBuffer;

		i32 CurrentPinFlags;
		ImVector<i32> pinFlagStack;

		// UI element state
		AST::Id currentNodeId;
		PinIdx CurrentPinIdx;
		i32 CurrentPinId;

		AST::Id hoveredNodeId;
		OptionalIndex HoveredLinkIdx;
		PinIdx HoveredPinIdx;

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

		bool leftMouseClicked;
		bool leftMouseReleased;
		bool altMouseClicked;
		bool leftMouseDragging;
		bool altMouseDragging;
		float altMouseScrollDelta;
		bool multipleSelectModifier;
	};


	// [SECTION] ObjectPool implementation

	template<typename T>
	static inline i32 ObjectPoolFind(const ObjectPool<T>& objects, const Id id)
	{
		return objects.idMap.GetInt(static_cast<ImGuiID>(id), -1);
	}

	template<typename T>
	static inline void ObjectPoolUpdate(ObjectPool<T>& objects)
	{
		for (i32 i = 0; i < objects.InUse.Size(); ++i)
		{
			const Id id = objects.Pool[i].id;

			if (!objects.InUse.IsSet(i) && objects.idMap.GetInt(id, -1) == i)
			{
				objects.idMap.SetInt(id, -1);
				objects.availableIds.Add(i);
				(objects.Pool.Data() + i)->~T();
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
		i32 index = objects.idMap.GetInt(static_cast<ImGuiID>(id), -1);

		// Construct new object
		if (index == -1)
		{
			if (objects.availableIds.IsEmpty())
			{
				index = objects.Pool.Size();
				Check(objects.Pool.Size() == objects.InUse.Size());
				const i32 newSize = objects.Pool.Size() + 1;
				objects.Pool.Resize(newSize);
				objects.InUse.Resize(newSize);
			}
			else
			{
				index = objects.availableIds.Last();
				objects.availableIds.RemoveLast();
			}
			objects.Pool[index] = T{id};
			objects.idMap.SetInt(static_cast<ImGuiID>(id), index);
		}

		// Flag it as used
		objects.InUse.FillBit(index);
		return index;
	}

	template<typename T>
	static inline T& ObjectPoolFindOrCreateObject(ObjectPool<T>& objects, const Id id)
	{
		const int index = ObjectPoolFindOrCreateIndex(objects, id);
		return objects.Pool[index];
	}
}    // namespace rift::Nodes
