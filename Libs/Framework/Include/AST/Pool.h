

#include "AST/Types.h"

#include <Containers/BitArray.h>
#include <Containers/Map.h>
#include <Containers/Set.h>
#include <Events/Broadcast.h>
#include <Memory/STLAllocator.h>
#include <TypeTraits.h>


namespace Rift::AST
{
	template<typename Allocator = Memory::DefaultAllocator>
	struct TPoolSet : public entt::basic_sparse_set<Id, STLAllocator<Id, Allocator>>
	{
		using Super = entt::basic_sparse_set<Id, STLAllocator<Id, Allocator>>;

	public:
		using Super::Super;

		void PopSwap(Id id)
		{
			Super::swap_and_pop(id, nullptr);
		}

		void Pop(Id id)
		{
			Super::in_place_pop(id, nullptr);
		}

		void Reserve(sizet capacity)
		{
			Super::reserve(capacity);
		}
	};


	template<typename T, typename Allocator>
	struct TPoolData
	{
		static constexpr sizet chunkSize = 1024;

		using AllocatorTraits = std::allocator_traits<STLAllocator<T, Allocator>>;
		static_assert(AllocatorTraits::propagate_on_container_move_assignment::value);
		using Chunk = T*;

		template<typename Value>
		struct TPoolIterator final
		{
			using difference_type   = typename IdTraits<Id>::Difference;
			using value_type        = Value;
			using pointer           = Value*;
			using reference         = Value&;
			using iterator_category = std::random_access_iterator_tag;

			TPoolIterator() = default;

			TPoolIterator(Value* const* chunks, const difference_type index)
			    : chunks{chunks}, index{index}
			{}

			TPoolIterator& operator++()
			{
				return --index, *this;
			}

			TPoolIterator operator++(int)
			{
				TPoolIterator orig = *this;
				return ++(*this), orig;
			}

			TPoolIterator& operator--()
			{
				return ++index, *this;
			}

			TPoolIterator operator--(int)
			{
				TPoolIterator orig = *this;
				return operator--(), orig;
			}

			TPoolIterator& operator+=(const difference_type value)
			{
				index -= value;
				return *this;
			}

			TPoolIterator operator+(const difference_type value) const
			{
				TPoolIterator copy = *this;
				return (copy += value);
			}

			TPoolIterator& operator-=(const difference_type value)
			{
				return (*this += -value);
			}

			TPoolIterator operator-(const difference_type value) const
			{
				return (*this + -value);
			}

			difference_type operator-(const TPoolIterator& other) const
			{
				return other.index - index;
			}

			reference operator[](const difference_type value) const
			{
				const sizet pos{index - value - 1};
				return (*chunks)[GetChunk(pos)][GetOffset(pos)];
			}

			bool operator==(const TPoolIterator& other) const
			{
				return other.index == index;
			}

			bool operator!=(const TPoolIterator& other) const
			{
				return !(*this == other);
			}

			auto operator<=>(const TPoolIterator& other) const
			{
				return other.index <=> index;
			}

			pointer operator->() const
			{
				const sizet pos{index - 1u};
				return std::addressof(chunks[GetChunk(pos)][GetOffset(pos)]);
			}

			reference operator*() const
			{
				const sizet pos{index - 1u};
				return chunks[GetChunk(pos)][GetOffset(pos)];
			}

		private:
			Value* const* chunks;
			difference_type index;
		};

		using Iterator = TPoolIterator<T>;
		/*! @brief Constant random access iterator type. */
		using ConstIterator = TPoolIterator<const T>;
		/*! @brief Reverse iterator type. */
		using ReverseIterator = std::reverse_iterator<Iterator>;
		/*! @brief Constant reverse iterator type. */
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;


	private:

		STLAllocator<T, Allocator> allocator;
		TArray<Chunk> chunks;
		sizet size = 0;


	public:
		T* Get(sizet index) const
		{
			return chunks[GetChunk(index)] + GetOffset(index);
		}

		void Reserve(sizet capacity)
		{
			const i32 neededChunks = GetChunk(capacity - 1) + 1;
			if (neededChunks <= chunks.Size())    // There are enough buckets
			{
				return;
			}

			chunks.Reserve(neededChunks);
			while (chunks.Size() < neededChunks)
			{
				chunks.Add(AllocatorTraits::allocate(allocator, chunkSize));
			}
		}

		// Release chunks to an specific size. No destructors are called
		void Release(sizet newSize)
		{
			CheckMsg(newSize >= size, "Cant erelease memory below used size");
			const i32 usedChunks = i32(newSize / chunkSize);
			if (usedChunks >= chunks.Size())
			{
				return;
			}

			for (auto pos = usedChunks; pos < chunks.Size(); ++pos)
			{
				AllocatorTraits::deallocate(allocator, chunks[pos], chunkSize);
			}
			chunks.RemoveLast(chunks.Size() - usedChunks);
		}

		// Release chunks to an specific size. No destructors are called
		void Reset()
		{
			for (Chunk chunk : chunks)
			{
				AllocatorTraits::deallocate(allocator, chunk, chunkSize);
			}
			chunks.Empty();
		}

		T* Push(sizet index, T&& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			if constexpr (std::is_aggregate_v<T>)
			{
				AllocatorTraits::construct(allocator, instance, Move(value));
			}
			else
			{
				AllocatorTraits::construct(allocator, instance, Move(value));
			}
			return instance;
		}

		T* Push(sizet index, const T& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			if constexpr (std::is_aggregate_v<T>)
			{
				AllocatorTraits::construct(allocator, instance, value);
			}
			else
			{
				AllocatorTraits::construct(allocator, instance, value);
			}
			return instance;
		}

		void PopSwap(sizet index, sizet last)
		{
			T& item     = chunks[GetChunk(index)][GetOffset(index)];
			T& lastItem = chunks[GetChunk(last)][GetOffset(last)];
			Swap(item, lastItem);
			AllocatorTraits::destroy(allocator, &lastItem);
			--size;
		}
		void Pop(sizet index)
		{
			T& item = chunks[GetChunk(index)][GetOffset(index)];
			AllocatorTraits::destroy(allocator, &item);
			--size;
		}

		static i32 GetChunk(sizet index)
		{
			return i32(index / chunkSize);
		}

		static sizet GetOffset(sizet index)
		{
			return index & (chunkSize - 1);
		}

		ConstIterator cbegin() const
		{
			const sizet index = size;
			return ConstIterator{chunks.Data(), index};
		}

		ConstIterator begin() const
		{
			return cbegin();
		}

		Iterator begin()
		{
			const sizet index = size;
			return Iterator{chunks.Data(), index};
		}
		Iterator end()
		{
			return iterator{chunks.Data(), {}};
		}
		Iterator end() const
		{
			return cend();
		}
		Iterator cend() const
		{
			return Iterator{chunks.Data(), {}};
		}

		ConstReverseIterator crbegin() const
		{
			return std::make_reverse_iterator(cend());
		}

		ConstReverseIterator rbegin() const
		{
			return crbegin();
		}

		ReverseIterator rbegin()
		{
			return std::make_reverse_iterator(end());
		}

		ConstReverseIterator crend() const
		{
			return std::make_reverse_iterator(cbegin());
		}

		ConstReverseIterator rend() const
		{
			return crend();
		}

		ReverseIterator rend()
		{
			return std::make_reverse_iterator(begin());
		}
	};


	template<typename T, typename Allocator>
		requires(IsEmpty<T>())
	struct TPoolData<T, Allocator>
	{
	public:
		T* Get(sizet index) const
		{
			return nullptr;
		}
		void Reserve(sizet size) {}
		void Release(sizet size) {}
		void Reset() {}
		T* Push(sizet index, const T&)
		{
			return nullptr;
		}
		T* Push(sizet index, T&&)
		{
			return nullptr;
		}
		void PopSwap(sizet index, sizet last) {}
		void Pop(sizet index) {}
	};


	struct Pool
	{
		enum class DeletionPolicy : u8
		{
			Swap,
			InPlace
		};

		using Index = IdTraits<Id>::Index;

		using Iterator        = TPoolSet<>::iterator;
		using ReverseIterator = TPoolSet<>::reverse_iterator;


	protected:
		TPoolSet<> set;
		DeletionPolicy deletionPolicy;

		TBroadcast<TArrayView<const Id>> onAdd;
		TBroadcast<TArrayView<const Id>> onRemove;


		Pool(DeletionPolicy inDeletionPolicy)
		    : set{entt::deletion_policy(u8(inDeletionPolicy))}, deletionPolicy{inDeletionPolicy}
		{}

	public:
		virtual ~Pool() {}

		bool Has(Id id) const
		{
			return set.contains(id);
		}

		virtual bool Remove(Id id)                          = 0;
		virtual void RemoveUnsafe(Id id)                    = 0;
		virtual i32 Remove(TArrayView<const Id> ids)        = 0;
		virtual void RemoveUnsafe(TArrayView<const Id> ids) = 0;

		virtual Pool* Clone() = 0;

		sizet Size() const
		{
			return set.size();
		}

		Iterator begin() const
		{
			return set.begin();
		}

		Iterator end() const
		{
			return set.end();
		}

		ReverseIterator rbegin() const
		{
			return set.rbegin();
		}

		ReverseIterator rend() const
		{
			return set.rend();
		}

		TBroadcast<TArrayView<const Id>>& OnAdd()
		{
			return onAdd;
		}

		TBroadcast<TArrayView<const Id>>& OnRemove()
		{
			return onRemove;
		}
	};


	template<typename T, typename Allocator = Memory::DefaultAllocator>
	struct TPool : public Pool
	{
	private:
		TPoolData<T, Allocator> data;


	public:
		TPool() : Pool(DeletionPolicy::InPlace) {}
		TPool(const TPool& other)
		{
			CopyFrom(other);
		}
		TPool(TPool&& other)
		{
			MoveFrom(Move(other));
		}
		TPool& operator=(const TPool& other)
		{
			CopyFrom(other);
		}
		TPool& operator=(TPool&& other)
		{
			MoveFrom(Move(other));
		}
		~TPool() override
		{
			set.clear();
			data.Reset();
		}

		void Add(Id id, const T&) requires(IsEmpty<T>())
		{
			set.emplace(id);
			onAdd.Broadcast({id});
		}

		T& Add(Id id, const T& v) requires(!IsEmpty<T>())
		{
			const auto i = set.slot();
			data.Reserve(i + 1u);

			T& value = *data.Push(i, v);

			const auto setI = set.emplace(id);
			if (!Ensure(i == setI)) [[unlikely]]
			{
				Log::Error("Misplaced component");
				data.Pop(i);
			}
			else
			{
				onAdd.Broadcast({id});
			}
			return value;
		}
		T& Add(Id id, T&& v = {}) requires(!IsEmpty<T>())
		{
			const auto i = set.slot();
			data.Reserve(i + 1u);

			T& value = *data.Push(i, Forward<T>(v));

			const auto setI = set.emplace(id);
			if (!Ensure(i == setI)) [[unlikely]]
			{
				Log::Error("Misplaced component");
				data.Pop(i);
			}
			else
			{
				onAdd.Broadcast({id});
			}
			return value;
		}

		template<typename It>
		void Add(It first, It last, const T& value = {})
		{
			const sizet numToAdd = std::distance(first, last);
			const sizet newSize  = set.size() + numToAdd;
			set.Reserve(newSize);
			data.Reserve(newSize);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			for (Id id : ids)
			{
				data.Push(set.size(), value);
				set.emplace_back(id);
			}
			onAdd.Broadcast(ids);
		}

		template<typename It, typename CIt>
		void Add(It first, It last, CIt from) requires(
		    IsSame<std::decay_t<typename std::iterator_traits<CIt>::value_type>, T>)
		{
			const sizet numToAdd = std::distance(first, last);
			const sizet newSize  = set.size() + numToAdd;
			set.Reserve(newSize);
			data.Reserve(newSize);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			for (Id id : ids)
			{
				data.Push(set.size(), *from);
				set.emplace_back(id);
				++from;
			}
			onAdd.Broadcast(ids);
		}

		T& GetOrAdd(const Id id) requires(!IsEmpty<T>())
		{
			return Has(id) ? Get(id) : Add(id);
		}

		bool Remove(Id id) override
		{
			if (Has(id))
			{
				RemoveUnsafe(id);
				return true;
			}
			return false;
		}

		void RemoveUnsafe(Id id) override
		{
			Check(Has(id));
			onRemove.Broadcast({id});
			if (deletionPolicy == DeletionPolicy::InPlace)
				Pop(id);
			else
				PopSwap(id);
		}

		i32 Remove(TArrayView<const Id> ids) override
		{
			i32 removed = 0;
			onRemove.Broadcast(ids);
			if (deletionPolicy == DeletionPolicy::InPlace)
			{
				for (Id id : ids)
				{
					if (Has(id))
					{
						Pop(id);
						++removed;
					}
				}
			}
			else
			{
				for (Id id : ids)
				{
					if (Has(id))
					{
						PopSwap(id);
						++removed;
					}
				}
			}
			return removed;
		}

		void RemoveUnsafe(TArrayView<const Id> ids) override
		{
			onRemove.Broadcast(ids);
			if (deletionPolicy == DeletionPolicy::InPlace)
			{
				for (Id id : ids)
				{
					Check(Has(id));
					Pop(id);
				}
			}
			else
			{
				for (Id id : ids)
				{
					Check(Has(id));
					PopSwap(id);
				}
			}
		}

		T& Get(Id id) requires(!IsEmpty<T>())
		{
			return *data.Get(set.index(id));
		}

		const T& Get(Id id) const requires(!IsEmpty<T>())
		{
			return *data.Get(set.index(id));
		}

		T* TryGet(Id id)
		{
			return Has(id) ? data.Get(set.index(id)) : nullptr;
		}

		const T* TryGet(Id id) const
		{
			return Has(id) ? data.Get(set.index(id)) : nullptr;
		}

		Pool* Clone() override
		{
			TPool<T>* newPool = new TPool<T>();

			if constexpr (IsEmpty<T>())
			{
				newPool->Add(set.begin(), set.end(), {});
			}
			else
			{
				newPool->Add(set.begin(), set.end(), data.begin());
			}
			return newPool;
		}

		void Reserve(sizet size)
		{
			set.Reserve(size);
			if (size > set.Size())
			{
				data.Reserve(size);
			}
		}

		void Shrink()
		{
			set.shrink_to_fit();
			data.Release(set.Size());
		}

	private:
		void CopyFrom(const TPool& other)
		{
			set = other.set;
			data.Reserve(set.Size());
			Add(other.set.begin(), other.set.end(), other.begin());
		}

		void MoveFrom(TPool&& other)
		{
			set = Move(other.set);
			data.Reset();

			// TODO: Move!
		}

		void PopSwap(Id id)
		{
			data.PopSwap(set.index(id), set.size() - 1u);
			set.PopSwap(id);
		}

		void Pop(Id id)
		{
			data.Pop(set.index(id));
			set.Pop(id);
		}
	};


	struct PoolInstance
	{
		Refl::TypeId componentId;

	private:
		Pool* instance = nullptr;


	public:
		PoolInstance(Refl::TypeId componentId) : componentId{componentId} {}
		PoolInstance(PoolInstance&& other)
		{
			componentId       = other.componentId;
			other.componentId = Refl::TypeId::None();
			instance       = Move(other.instance);
			other.instance = nullptr;
		}
		PoolInstance& operator=(PoolInstance&& other)
		{
			componentId       = other.componentId;
			other.componentId = Refl::TypeId::None();
			instance       = Move(other.instance);
			other.instance = nullptr;
			return *this;
		}
		explicit PoolInstance(const PoolInstance& other)
		{
			componentId = other.componentId;
			if (other.instance)
			{
				instance = other.instance->Clone();
			}
		}

		Pool* GetInstance() const
		{
			return instance;
		}

		~PoolInstance()
		{
			delete instance;
		}
		bool operator<(const PoolInstance& other) const
		{
			return componentId.GetId() < other.componentId.GetId();
		}

		template<typename T>
		void Create()
		{
			Check(!instance);
			instance = new TPool<std::remove_const_t<T>>();
			// TODO: Static inheritance methods
		}
	};
}    // namespace Rift::AST
