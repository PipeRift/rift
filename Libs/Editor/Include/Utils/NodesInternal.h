// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "UI/UIImgui.h"
#include "Utils/Nodes.h"
#include "Utils/NodesMiniMap.h"

#include <assert.h>
#include <AST/Pool.h>
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

	using UIState              = int;
	using ClickInteractionType = int;
	using LinkCreationType     = int;

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
		ImGuiStorage idMap;
	};

	template<typename T>
	struct IndexedArray
	{
		TArray<AST::Id> used;
		TArray<T> data;

		struct TIterator final
		{
			using difference_type   = typename IdTraits<Id>::Difference;
			using value_type        = Value;
			using pointer           = Value*;
			using reference         = Value&;
			using iterator_category = std::random_access_iterator_tag;

		private:
			difference_type index = NO_INDEX;
			AST::Id* used         = nullptr;
			u32 size              = 0;


		public:
			Iterator() = default;
			Iterator(const difference_type index, TArray<AST::Id>& used)
			    : index{index}, used{used.Data()}, size{used.Size()}
			{}

			Iterator& operator++()
			{
				while (++index < size && !IsValid()) {}
				return *this;
			}
			Iterator& operator--()
			{
				while (--index >= 0 && !IsValid()) {}
				return *this;
			}
			Iterator operator++(int)
			{
				Iterator orig = *this;
				return ++(*this), orig;
			}
			Iterator operator--(int)
			{
				Iterator orig = *this;
				return --(*this), orig;
			}

			bool operator==(const Iterator& other) const
			{
				return other.index == index;
			}
			bool operator!=(const Iterator& other) const
			{
				return !(*this == other);
			}
			auto operator<=>(const Iterator& other) const
			{
				return other.index <=> index;
			}

			pointer operator->() const
			{
				return used + index;
			}

			reference operator*() const
			{
				return used[index];
			}

		private:
			bool IsValid() const
			{
				const AST::Id id = operator*();
				return AST::GetVersion(id) != AST::GetVersion(AST::NoId);
			}
		};

		T& GetOrAdd(AST::Id id, bool* outAdded = nullptr)
		{
			const u32 index = AST::GetIndex(id);
			if (used.Size() <= index)
			{
				used.Resize(index + 1);
				data.Resize(index + 1);

				if (outAdded)
				{
					*outAdded = true;
				}
			}
			T* const ptr = GetByIndex(index);
			if (!used.IsSet(index))
			{
				used.FillBit(index);
				*ptr = {};
			}
			return *ptr;
		}

		T& Get(AST::Id id)
		{
			const u32 index = AST::GetIndex(id);
			Check(index < used.Size() && used.IsSet(index));
			return *GetByIndex(index);
		}

		const T& Get(AST::Id id) const
		{
			return const_cast<IndexedArray<T>*>(this)->Get(id);
		}

		T* TryGet(AST::Id id)
		{
			const u32 index = AST::GetIndex(id);
			if (index < used.Size() && used.IsSet(index))
			{
				return GetByIndex(index);
			}
			return nullptr;
		}

		bool Contains(AST::Id id) const
		{
			const u32 index = AST::GetIndex(id);
			return index < used.Size() && used.IsSet(index);
		}

		T& operator[](AST::Id id)
		{
			return Get(id);
		}

		void Reset(u32 size)
		{
			used.Reset(size);
		}

		Iterator begin() const
		{
			return {0, iterablePool->begin(), access.GetPoolArray(iterablePool),
			    access.GetExcludedPoolArray()};
		}

		Iterator end() const
		{
			return {iterablePool->begin(), iterablePool->end(), iterablePool->end(),
			    access.GetPoolArray(iterablePool), access.GetExcludedPoolArray()};
		}

	private:
		T* GetByIndex(u32 index)
		{
			return data[index];
		}
	};

	struct DepthVertex
	{
		AST::Id front = AST::NoId;
		AST::Id back  = AST::NoId;
	};
	template<typename T>
	struct NodeArray : public IndexedArray<T>
	{
		TArray<AST::Id> depthOrder;


		T& GetOrAdd(AST::Id id, bool* outAdded = nullptr)
		{
			bool added = false;
			T& node    = IndexedArray<T>::GetOrAdd(id, &added);
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


		NodeData(const Id id = NoId()) : id(id) {}
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

		ImVector<AST::Id> nodeDepthOrder;

		// ui related fields
		v2 Panning = v2::Zero();
		v2 AutoPanningDelta;
		// Minimum and maximum extents of all content in grid space. Valid after final
		// Nodes::EndNode() call.
		Rect gridContentBounds;

		ImVector<AST::Id> selectedNodeIds;
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
		ImVector<i32> NodeIdxSubmissionOrder;
		ImVector<i32> NodeIndicesOverlappingWithMouse;
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
}    // namespace Rift::Nodes
