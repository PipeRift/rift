// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "UI/UIImgui.h"
#include "Utils/Nodes.h"
#include "Utils/NodesMiniMap.h"

#include <assert.h>
#include <AST/Id.h>
#include <limits.h>
#include <Pipe/Core/EnumFlags.h>
#include <PipeContainers.h>
#include <PipeMath.h>


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
	P_DEFINE_FLAG_OPERATORS(Scope);

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

	// T must be default constructible and hold an i32 id
	template<typename T>
	struct ObjectPool
	{
		TArray<T> pool;
		BitArray inUse;
		TArray<i32> availableIds;
		ImGuiStorage idMap;
	};

	template<typename T>
	struct TIndexedArray
	{
		using Iterator      = TArray<ast::Id>::Iterator;
		using ConstIterator = TArray<ast::Id>::ConstIterator;


		TArray<ast::Id> frameIds;
		TArray<T> data;
		TArray<ast::Id> lastFrameIds;
		TArray<ast::Id> invalidIds;


	public:
		T& GetOrAdd(ast::Id id, bool* outAdded = nullptr)
		{
			const u32 index = id.GetIndex();
			bool added;
			frameIds.AddUniqueSorted(id, {}, &added);
			if (added && !lastFrameIds.ContainsSorted(id))
			{
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

		T& Get(ast::Id id)
		{
			return *GetByIndex(id.GetIndex());
		}

		const T& Get(ast::Id id) const
		{
			return const_cast<TIndexedArray<T>*>(this)->Get(id);
		}

		T* TryGet(ast::Id id)
		{
			if (Contains(id))
			{
				return GetByIndex(id.GetIndex());
			}
			return nullptr;
		}

		bool Contains(ast::Id id) const
		{
			return frameIds.ContainsSorted(id);
		}

		T& operator[](ast::Id id)
		{
			return Get(id);
		}
		const T& operator[](ast::Id id) const
		{
			return Get(id);
		}

		void CacheInvalidIds()
		{
			invalidIds.Clear(p::Shrink::No);
			for (ast::Id id : lastFrameIds)
			{
				if (!frameIds.ContainsSorted(id))
				{
					invalidIds.Add(id);
				}
			}
		}

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
			P_Check(data.IsValidIndex(index));
			return data.Data() + index;
		}
	};

	template<typename T>
	struct NodeArray : public TIndexedArray<T>
	{
		TArray<ast::Id> depthOrder;


		T& GetOrAdd(ast::Id id, bool* outAdded = nullptr)
		{
			bool added = false;
			T& node    = TIndexedArray<T>::GetOrAdd(id, &added);
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

		void PushToTheFront(ast::Id id)
		{
			depthOrder.Remove(id, p::Shrink::No);
			depthOrder.Add(id);
		}

		void ClearDepthOrder()
		{
			depthOrder.RemoveIf([this](ast::Id id)
			{
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


	// Rows and columns of the table a node is laid out as
	enum NodeRow_
	{
		NodeRow_Header = 0,
		NodeRow_Body,
		NodeRow_COUNT
	};

	enum NodeColumn_
	{
		NodeColumn_Inputs = 0,
		NodeColumn_Name,
		NodeColumn_Outputs,
		NodeColumn_COUNT
	};


	struct NodeData
	{
		v2 Origin = v2::Zero();    // The node origin is in editor space
		Rect rect{v2::Zero(), v2::Zero()};

		// Measurements of the node's cells on the last frame, used to lay it out
		v2 cellContentSize[NodeRow_COUNT][NodeColumn_COUNT]{};
		float blockHeight[NodeRow_COUNT][NodeColumn_COUNT]{};
		float rowHeight[NodeRow_COUNT]{};
		float nodeWidth = 0.f;
		// The zoom the measurements were taken at, so that they can be scaled back
		float contentZoom = 0.f;

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


	// Table layout of the node being submitted, valid between BeginNode() and EndNode()
	struct NodeLayout
	{
		// Row and column being submitted
		NodeRow_ row       = NodeRow_Body;
		NodeColumn_ column = NodeColumn_Name;
		bool cellOpen      = false;
		// True between BeginNode() and EndNode(), so that a leaked node can be cleaned up
		bool nodeOpen = false;
		// Depth of the ImGui group stack when the node started
		i32 groupDepth = 0;

		// Screen space position of the node's top left corner and padding of every cell
		v2 origin  = v2::Zero();
		v2 padding = v2::Zero();
		// The width of the node, from the last frame
		float nodeWidth = 0.f;

		// Measurements of each cell, this frame and the one before. The last one lays the
		// node out, the current one is picked up by the next frame.
		v2 cellContentSize[NodeRow_COUNT][NodeColumn_COUNT]     = {};
		v2 prevCellContentSize[NodeRow_COUNT][NodeColumn_COUNT] = {};
		float blockHeight[NodeRow_COUNT][NodeColumn_COUNT]      = {};
		float prevBlockHeight[NodeRow_COUNT][NodeColumn_COUNT]  = {};
		// How far the next cell of each column has stacked in the row being submitted
		float cellY[NodeColumn_COUNT] = {};
	};


	struct PinData
	{
		Id id;
		ast::Id parentNodeId = ast::NoId;
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


	// Visual overlay indicating the current canvas zoom. It stays hidden until the mouse
	// hovers over it or the zoom changes, in which case it is shown for a short while.
	struct ZoomIndicator
	{
		// Screen-space rectangle of the indicator, valid while it is drawn.
		Rect rectScreenSpace;
		// Time at which the indicator stops being visible after the last zoom change.
		double visibleEndTime = 0.0;
		// Current fade alpha in [0, 1].
		float alpha = 0.f;
		// Zoom observed on the previous frame, used to detect changes.
		float lastZoom = 1.f;

		bool hovered  = false;
		bool dragging = false;

		void Update();
		void Draw();
	};


	// A node dragged along with the primary node, and its origin relative to it
	struct NodeDragOrigin
	{
		ast::Id id;
		v2 origin;
	};

	struct EditorContext
	{
		NodeArray<NodeData> nodes;
		ObjectPool<PinData> outputs;
		ObjectPool<PinData> inputs;
		ObjectPool<LinkData> links;

		// ui related fields
		v2 panning = v2::Zero();
		v2 AutoPanningDelta;
		// Minimum and maximum extents of all content in grid space. Valid after final
		// Nodes::EndNode() call.
		Rect gridContentBounds;

		TArray<ast::Id> selectedNodeIds;
		ImVector<i32> selectedLinkIndices;

		// Nodes dragged along with the primary node, recorded when the drag starts
		ImVector<NodeDragOrigin> DragOrigins;
		// Offset of the primary node origin relative to the mouse cursor.
		v2 PrimaryNodeOffset;

		ClickInteractionState clickInteraction;

		MiniMap miniMap;

		ZoomIndicator zoomIndicator;

		bool canCreateLinks = true;


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
			return GetPinPool(pin.type).pool[pin.index];
		}
		const PinData& GetPinData(PinIdx pin) const
		{
			// TODO: Incompatible id types!
			return GetPinPool(pin.type).pool[i32(pin.index)];
		}
	};

	struct Context
	{
		EditorContext* DefaultEditorCtx;
		EditorContext* EditorCtx;

		// Canvas draw list and helper state
		ImDrawList* CanvasDrawList;
		ImGuiStorage NodeIdxToSubmissionIdx;
		ImVector<ast::Id> nodeSubmissionOrder;
		ImVector<ast::Id> nodeIdsOverlappingWithMouse;
		// Channel where every node pin is drawn, above all the node channels
		i32 pinChannelIdx = -1;

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
		ast::Id currentNodeId;
		NodeLayout nodeLayout;
		PinIdx CurrentPinIdx;
		i32 CurrentPinId;

		ast::Id hoveredNodeId;
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

		bool canCreateLinks = true;
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
		for (i32 i = 0; i < objects.inUse.Size(); ++i)
		{
			if (!objects.inUse.IsSet(i))
			{
				const Id id = objects.pool[i].id;
				if (objects.idMap.GetInt(id, -1) == i)
				{
					objects.idMap.SetInt(id, -1);
					objects.availableIds.Add(i);
					(objects.pool.Data() + i)->~T();
				}
			}
		}
	}

	template<typename T>
	static inline void ObjectPoolReset(ObjectPool<T>& objects)
	{
		if (!objects.inUse.IsEmpty())
		{
			objects.inUse.SetAllFalse();
		}
	}

	template<typename T>
	static inline i32 ObjectPoolFindOrCreateIndex(ObjectPool<T>& objects, const i32 id)
	{
		i32 index = objects.idMap.GetInt(static_cast<ImGuiID>(id), -1);

		if (index == -1)
		{
			if (objects.availableIds.IsEmpty())
			{
				index = objects.pool.Size();
				P_Check(objects.pool.Size() == objects.inUse.Size());
				const i32 newSize = objects.pool.Size() + 1;
				objects.pool.Resize(newSize);
				objects.inUse.Resize(newSize);
			}
			else
			{
				index = objects.availableIds.Last();
				objects.availableIds.RemoveLast();
			}
			objects.pool[index] = T{id};
			objects.idMap.SetInt(static_cast<ImGuiID>(id), index);
		}

		objects.inUse.SetTrue(index);
		return index;
	}

	template<typename T>
	static inline T& ObjectPoolFindOrCreateObject(ObjectPool<T>& objects, const Id id)
	{
		const int index = ObjectPoolFindOrCreateIndex(objects, id);
		return objects.pool[index];
	}
}    // namespace rift::Nodes
