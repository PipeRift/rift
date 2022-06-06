// Copyright 2015-2022 Piperift - All rights reserved
// Modified version of sparse_set.hpp from entt
// https://github.com/skypjack/entt

#pragma once

#include "AST/Id.h"

#include <PCore/Checks.h>

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>


//#include "../config/config.h"
//#include "../core/algorithm.hpp"
//#include "../core/fwd.hpp"
//#include "entity.hpp"
//#include "fwd.hpp"


namespace rift::AST
{
	enum class DeletionPolicy : u8
	{
		Swap,
		InPlace
	};


	/**
	 * @brief Basic sparse set implementation.
	 *
	 * Sparse set or packed array or whatever is the name users give it.<br/>
	 * Two arrays: an _external_ one and an _internal_ one; a _sparse_ one and a
	 * _packed_ one; one used for direct access through contiguous memory, the other
	 * one used to get the data through an extra level of indirection.<br/>
	 * This is largely used by the registry to offer users the fastest access ever
	 * to the components. Views and groups in general are almost entirely designed
	 * around sparse sets.
	 *
	 * This type of data structure is widely documented in the literature and on the
	 * web. This is nothing more than a customized implementation suitable for the
	 * purpose of the framework.
	 *
	 * @note
	 * Internal data structures arrange elements to maximize performance. There are
	 * no guarantees that entities are returned in the insertion order when iterate
	 * a sparse set. Do not make assumption on the order in any case.
	 *
	 * @tparam AST::Id A valid entity type (see entt_traits for more details).
	 * @tparam Allocator Type of allocator used to manage memory and elements.
	 */
	template<typename Allocator>
	class BasicSparseSet
	{
		static constexpr auto growthFactor = 1.5;
		static constexpr auto sparsePage   = 4096;

		using traits_type = AST::IdTraits<AST::Id>;

		using alloc_traits =
		    typename std::allocator_traits<Allocator>::template rebind_traits<AST::Id>;
		using alloc_pointer       = typename alloc_traits::pointer;
		using alloc_const_pointer = typename alloc_traits::const_pointer;

		using bucket_alloc_traits =
		    typename std::allocator_traits<Allocator>::template rebind_traits<alloc_pointer>;
		using bucket_alloc_pointer = typename bucket_alloc_traits::pointer;

		static_assert(alloc_traits::propagate_on_container_move_assignment::value);
		static_assert(bucket_alloc_traits::propagate_on_container_move_assignment::value);

		struct SparseSetIterator final
		{
			using difference_type   = typename traits_type::Difference;
			using value_type        = AST::Id;
			using pointer           = const value_type*;
			using reference         = const value_type&;
			using iterator_category = std::random_access_iterator_tag;

			SparseSetIterator() = default;

			SparseSetIterator(const alloc_const_pointer* ref, const difference_type idx)
			    : packed{ref}, index{idx}
			{}

			SparseSetIterator& operator++()
			{
				return --index, *this;
			}

			SparseSetIterator operator++(int)
			{
				iterator orig = *this;
				return ++(*this), orig;
			}

			SparseSetIterator& operator--()
			{
				return ++index, *this;
			}

			SparseSetIterator operator--(int)
			{
				SparseSetIterator orig = *this;
				return operator--(), orig;
			}

			SparseSetIterator& operator+=(const difference_type value)
			{
				index -= value;
				return *this;
			}

			SparseSetIterator operator+(const difference_type value) const
			{
				SparseSetIterator copy = *this;
				return (copy += value);
			}

			SparseSetIterator& operator-=(const difference_type value)
			{
				return (*this += -value);
			}

			SparseSetIterator operator-(const difference_type value) const
			{
				return (*this + -value);
			}

			difference_type operator-(const SparseSetIterator& other) const
			{
				return other.index - index;
			}

			[[nodiscard]] reference operator[](const difference_type value) const
			{
				const auto pos = size_type(index - value - 1u);
				return (*packed)[pos];
			}

			[[nodiscard]] bool operator==(const SparseSetIterator& other) const
			{
				return other.index == index;
			}

			[[nodiscard]] bool operator!=(const SparseSetIterator& other) const
			{
				return !(*this == other);
			}

			[[nodiscard]] bool operator<(const SparseSetIterator& other) const
			{
				return index > other.index;
			}

			[[nodiscard]] bool operator>(const SparseSetIterator& other) const
			{
				return index < other.index;
			}

			[[nodiscard]] bool operator<=(const SparseSetIterator& other) const
			{
				return !(*this > other);
			}

			[[nodiscard]] bool operator>=(const SparseSetIterator& other) const
			{
				return !(*this < other);
			}

			[[nodiscard]] pointer operator->() const
			{
				const auto pos = size_type(index - 1u);
				return std::addressof((*packed)[pos]);
			}

			[[nodiscard]] reference operator*() const
			{
				return *operator->();
			}

		private:
			const alloc_const_pointer* packed;
			difference_type index;
		};

		[[nodiscard]] static auto Page(const AST::Id entt)
		{
			return static_cast<size_type>(ecs::GetIndex(entt) / sparsePage);
		}

		[[nodiscard]] static auto Offset(const AST::Id entt)
		{
			return static_cast<size_type>(ecs::GetIndex(entt) & (sparsePage - 1));
		}

		[[nodiscard]] auto AssurePage(const std::size_t idx)
		{
			if (!(idx < bucket))
			{
				const size_type sz = idx + 1u;
				const auto mem     = bucket_alloc_traits::allocate(bucketAllocator, sz);

				std::uninitialized_value_construct(mem + bucket, mem + sz);
				std::uninitialized_copy(sparse, sparse + bucket, mem);

				std::destroy(sparse, sparse + bucket);
				bucket_alloc_traits::deallocate(bucketAllocator, sparse, bucket);

				sparse = mem;
				bucket = sz;
			}

			if (!sparse[idx])
			{
				sparse[idx] = alloc_traits::allocate(allocator, sparsePage);
				std::uninitialized_fill(sparse[idx], sparse[idx] + sparsePage, AST::NoId);
			}

			return sparse[idx];
		}

		void ResizePacked(const std::size_t req)
		{
			CheckMsg((req != reserved) && !(req < count), "Invalid request");
			const auto mem = alloc_traits::allocate(allocator, req);

			std::uninitialized_copy(packed, packed + count, mem);
			std::uninitialized_fill(mem + count, mem + req, AST::NoId);

			std::destroy(packed, packed + reserved);
			alloc_traits::deallocate(allocator, packed, reserved);

			packed   = mem;
			reserved = req;
		}

		void ReleaseMemory()
		{
			if (packed)
			{
				for (size_type pos{}; pos < bucket; ++pos)
				{
					if (sparse[pos])
					{
						std::destroy(sparse[pos], sparse[pos] + sparsePage);
						alloc_traits::deallocate(allocator, sparse[pos], sparsePage);
					}
				}

				std::destroy(packed, packed + reserved);
				std::destroy(sparse, sparse + bucket);
				alloc_traits::deallocate(allocator, packed, reserved);
				bucket_alloc_traits::deallocate(bucketAllocator, sparse, bucket);
			}
		}

	protected:
		/**
		 * @brief Swaps two entities in the internal packed array.
		 * @param lhs A valid position of an entity within storage.
		 * @param rhs A valid position of an entity within storage.
		 */
		virtual void SwapAt(
		    [[maybe_unused]] const std::size_t lhs, [[maybe_unused]] const std::size_t rhs)
		{}

		/**
		 * @brief Moves an entity in the internal packed array.
		 * @param from A valid position of an entity within storage.
		 * @param to A valid position of an entity within storage.
		 */
		virtual void MoveAndPop(
		    [[maybe_unused]] const std::size_t from, [[maybe_unused]] const std::size_t to)
		{}

		/**
		 * @brief Attempts to erase an entity from the internal packed array.
		 * @param entt A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		virtual void SwapAndPop(const AST::Id entt, [[maybe_unused]] void* ud)
		{
			auto& ref      = sparse[Page(entt)][Offset(entt)];
			const auto pos = static_cast<size_type>(ecs::GetIndex(ref));
			CheckMsg(packed[pos] == entt, "Invalid entity identifier");
			auto& last = packed[--count];

			packed[pos]                      = last;
			sparse[Page(last)][Offset(last)] = ref;
			// lazy self-assignment guard
			ref = AST::NoId;
			// unnecessary but it helps to detect nasty bugs
			CheckMsg((last = AST::NoId, true), "");
		}

		/**
		 * @brief Attempts to erase an entity from the internal packed array.
		 * @param entt A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		virtual void InPlacePop(const AST::Id entt, [[maybe_unused]] void* ud)
		{
			auto& ref      = sparse[Page(entt)][Offset(entt)];
			const auto pos = static_cast<size_type>(ecs::GetIndex(ref));
			CheckMsg(packed[pos] == entt, "Invalid entity identifier");

			packed[pos] = std::exchange(
			    freeList, AST::MakeId(static_cast<typename traits_type::Entity>(pos)));
			// lazy self-assignment guard
			ref = AST::NoId;
		}

	public:
		/*! @brief Allocator type. */
		using allocator_type = typename alloc_traits::allocator_type;
		/*! @brief Underlying entity identifier. */
		using entity_type = AST::Id;
		/*! @brief Unsigned integer type. */
		using size_type = std::size_t;
		/*! @brief Pointer type to contained entities. */
		using pointer = alloc_const_pointer;
		/*! @brief Random access iterator type. */
		using iterator = SparseSetIterator;
		/*! @brief Reverse iterator type. */
		using reverse_iterator = std::reverse_iterator<iterator>;

		/**
		 * @brief Constructs an empty container with the given policy and allocator.
		 * @param pol Type of deletion policy.
		 * @param alloc Allocator to use (possibly default-constructed).
		 */
		explicit BasicSparseSet(DeletionPolicy pol, const allocator_type& alloc = {})
		    : allocator{alloc}
		    , bucketAllocator{alloc}
		    , sparse{bucket_alloc_traits::allocate(bucketAllocator, 0u)}
		    , packed{alloc_traits::allocate(allocator, 0u)}
		    , bucket{0u}
		    , count{0u}
		    , reserved{0u}
		    , freeList{AST::NoId}
		    , mode{pol}
		{}

		/**
		 * @brief Constructs an empty container with the given allocator.
		 * @param alloc Allocator to use (possibly default-constructed).
		 */
		explicit BasicSparseSet(const allocator_type& alloc = {})
		    : BasicSparseSet{DeletionPolicy::Swap, alloc}
		{}

		/**
		 * @brief Move constructor.
		 * @param other The instance to move from.
		 */
		BasicSparseSet(BasicSparseSet&& other) noexcept
		    : allocator{std::move(other.allocator)}
		    , bucketAllocator{std::move(other.bucketAllocator)}
		    , sparse{std::exchange(other.sparse, bucket_alloc_pointer{})}
		    , packed{std::exchange(other.packed, alloc_pointer{})}
		    , bucket{std::exchange(other.bucket, 0u)}
		    , count{std::exchange(other.count, 0u)}
		    , reserved{std::exchange(other.reserved, 0u)}
		    , freeList{std::exchange(other.freeList, AST::NoId)}
		    , mode{other.mode}
		{}

		/*! @brief Default destructor. */
		virtual ~BasicSparseSet()
		{
			ReleaseMemory();
		}

		/**
		 * @brief Move assignment operator.
		 * @param other The instance to move from.
		 * @return This sparse set.
		 */
		BasicSparseSet& operator=(BasicSparseSet&& other) noexcept
		{
			ReleaseMemory();

			allocator       = std::move(other.allocator);
			bucketAllocator = std::move(other.bucketAllocator);
			sparse          = std::exchange(other.sparse, bucket_alloc_pointer{});
			packed          = std::exchange(other.packed, alloc_pointer{});
			bucket          = std::exchange(other.bucket, 0u);
			count           = std::exchange(other.count, 0u);
			reserved        = std::exchange(other.reserved, 0u);
			freeList        = std::exchange(other.freeList, AST::NoId);
			mode            = other.mode;

			return *this;
		}

		/**
		 * @brief Returns the deletion policy of a sparse set.
		 * @return The deletion policy of the sparse set.
		 */
		[[nodiscard]] DeletionPolicy Policy() const
		{
			return mode;
		}

		/**
		 * @brief Returns the next slot available for insertion.
		 * @return The next slot available for insertion.
		 */
		[[nodiscard]] size_type Slot() const
		{
			return freeList == AST::NoId ? count : static_cast<size_type>(ecs::GetIndex(freeList));
		}

		/**
		 * @brief Increases the capacity of a sparse set.
		 *
		 * If the new capacity is greater than the current capacity, new storage is
		 * allocated, otherwise the method does nothing.
		 *
		 * @param cap Desired capacity.
		 */
		void Reserve(const size_type cap)
		{
			if (cap > reserved)
			{
				ResizePacked(cap);
			}
		}

		/**
		 * @brief Returns the number of elements that a sparse set has currently
		 * allocated space for.
		 * @return Capacity of the sparse set.
		 */
		[[nodiscard]] size_type Capacity() const
		{
			return reserved;
		}

		/*! @brief Requests the removal of unused capacity. */
		void ShrinkToFit()
		{
			if (count < reserved)
			{
				ResizePacked(count);
			}
		}

		/**
		 * @brief Returns the extent of a sparse set.
		 *
		 * The extent of a sparse set is also the size of the internal sparse array.
		 * There is no guarantee that the internal packed array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 *
		 * @return Extent of the sparse set.
		 */
		[[nodiscard]] size_type Extent() const
		{
			return bucket * sparsePage;
		}

		/**
		 * @brief Returns the number of elements in a sparse set.
		 *
		 * The number of elements is also the size of the internal packed array.
		 * There is no guarantee that the internal sparse array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 *
		 * @return Number of elements.
		 */
		[[nodiscard]] size_type Size() const
		{
			return count;
		}

		/**
		 * @brief Checks whether a sparse set is empty.
		 * @return True if the sparse set is empty, false otherwise.
		 */
		[[nodiscard]] bool Empty() const
		{
			return (count == size_type{});
		}

		/**
		 * @brief Direct access to the internal packed array.
		 * @return A pointer to the internal packed array.
		 */
		[[nodiscard]] pointer Data() const
		{
			return packed;
		}

		/**
		 * @brief Returns an iterator to the beginning.
		 *
		 * The returned iterator points to the first entity of the internal packed
		 * array. If the sparse set is empty, the returned iterator will be equal to
		 * `end()`.
		 *
		 * @return An iterator to the first entity of the internal packed array.
		 */
		[[nodiscard]] iterator begin() const
		{
			return iterator{
			    std::addressof(packed), static_cast<typename traits_type::Difference>(count)};
		}

		/**
		 * @brief Returns an iterator to the end.
		 *
		 * The returned iterator points to the element following the last entity in
		 * the internal packed array. Attempting to dereference the returned
		 * iterator results in undefined behavior.
		 *
		 * @return An iterator to the element following the last entity of the
		 * internal packed array.
		 */
		[[nodiscard]] iterator end() const
		{
			return iterator{std::addressof(packed), {}};
		}

		/**
		 * @brief Returns a reverse iterator to the beginning.
		 *
		 * The returned iterator points to the first entity of the reversed internal
		 * packed array. If the sparse set is empty, the returned iterator will be
		 * equal to `rend()`.
		 *
		 * @return An iterator to the first entity of the reversed internal packed
		 * array.
		 */
		[[nodiscard]] reverse_iterator rbegin() const
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Returns a reverse iterator to the end.
		 *
		 * The returned iterator points to the element following the last entity in
		 * the reversed internal packed array. Attempting to dereference the
		 * returned iterator results in undefined behavior.
		 *
		 * @return An iterator to the element following the last entity of the
		 * reversed internal packed array.
		 */
		[[nodiscard]] reverse_iterator rend() const
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @brief Finds an entity.
		 * @param entt A valid entity identifier.
		 * @return An iterator to the given entity if it's found, past the end
		 * iterator otherwise.
		 */
		[[nodiscard]] iterator Find(const entity_type entt) const
		{
			return Contains(entt) ? --(end() - Index(entt)) : end();
		}

		/**
		 * @brief Checks if a sparse set contains an entity.
		 * @param entt A valid entity identifier.
		 * @return True if the sparse set contains the entity, false otherwise.
		 */
		[[nodiscard]] bool Contains(const entity_type entt) const
		{
			//  Testing versions permits to avoid accessing the packed array
			const auto curr = Page(entt);
			return ecs::GetVersion(entt) != ecs::GetVersion(AST::NoId)
			    && (curr < bucket && sparse[curr] && sparse[curr][Offset(entt)] != AST::NoId);
		}

		/**
		 * @brief Returns the position of an entity in a sparse set.
		 *
		 * @warning
		 * Attempting to get the position of an entity that doesn't belong to the
		 * sparse set results in undefined behavior.
		 *
		 * @param entt A valid entity identifier.
		 * @return The position of the entity in the sparse set.
		 */
		[[nodiscard]] size_type Index(const entity_type entt) const
		{
			CheckMsg(Contains(entt), "Set does not contain entity");
			return static_cast<size_type>(ecs::GetIndex(sparse[Page(entt)][Offset(entt)]));
		}

		/**
		 * @brief Returns the entity at specified location, with bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location if any, a nullptr entity otherwise.
		 */
		[[nodiscard]] entity_type At(const size_type pos) const
		{
			return pos < count ? packed[pos] : nullptr;
		}

		/**
		 * @brief Returns the entity at specified location, without bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location.
		 */
		[[nodiscard]] entity_type operator[](const size_type pos) const
		{
			CheckMsg(pos < count, "Position is out of bounds");
			return packed[pos];
		}

		/**
		 * @brief Appends an entity to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @param entt A valid entity identifier.
		 * @return The slot used for insertion.
		 */
		size_type EmplaceBack(const entity_type entt)
		{
			CheckMsg(!Contains(entt), "Set already contains entity");

			if (count == reserved)
			{
				const auto sz = static_cast<size_type>(reserved * growthFactor);
				ResizePacked(sz + !(sz > reserved));
			}

			AssurePage(Page(entt))[Offset(entt)] =
			    AST::MakeId(static_cast<typename traits_type::Entity>(count));
			packed[count] = entt;
			return count++;
		}

		/**
		 * @brief Assigns an entity to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @param entt A valid entity identifier.
		 * @return The slot used for insertion.
		 */
		size_type Emplace(const entity_type entt)
		{
			if (freeList == AST::NoId)
			{
				return EmplaceBack(entt);
			}
			else
			{
				CheckMsg(!Contains(entt), "Set already contains entity");
				const auto pos = static_cast<size_type>(ecs::GetIndex(freeList));
				AssurePage(Page(entt))[Offset(entt)] =
				    AST::MakeId(static_cast<typename traits_type::Entity>(pos));
				freeList = std::exchange(packed[pos], entt);
				return pos;
			}
		}

		/**
		 * @brief Assigns one or more entities to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 */
		template<typename It>
		void Insert(It first, It last)
		{
			Reserve(count + std::distance(first, last));

			for (; first != last; ++first)
			{
				const auto entt = *first;
				CheckMsg(!Contains(entt), "Set already contains entity");
				AssurePage(Page(entt))[Offset(entt)] =
				    AST::MakeId(static_cast<typename traits_type::Entity>(count));
				packed[count++] = entt;
			}
		}

		/**
		 * @brief Erases an entity from a sparse set.
		 *
		 * @warning
		 * Attempting to erase an entity that doesn't belong to the sparse set
		 * results in undefined behavior.
		 *
		 * @param entt A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		void Erase(const entity_type entt, void* ud = nullptr)
		{
			CheckMsg(Contains(entt), "Set does not contain entity");
			(mode == DeletionPolicy::InPlace) ? InPlacePop(entt, ud) : SwapAndPop(entt, ud);
		}

		/**
		 * @brief Erases entities from a set.
		 *
		 * @sa erase
		 *
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		template<typename It>
		void Erase(It first, It last, void* ud = nullptr)
		{
			for (; first != last; ++first)
			{
				Erase(*first, ud);
			}
		}

		/**
		 * @brief Removes an entity from a sparse set if it exists.
		 * @param entt A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 * @return True if the entity is actually removed, false otherwise.
		 */
		bool Remove(const entity_type entt, void* ud = nullptr)
		{
			return Contains(entt) && (Erase(entt, ud), true);
		}

		/**
		 * @brief Removes entities from a sparse set if they exist.
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 * @return The number of entities actually removed.
		 */
		template<typename It>
		size_type Remove(It first, It last, void* ud = nullptr)
		{
			size_type found{};

			for (; first != last; ++first)
			{
				found += Remove(*first, ud);
			}

			return found;
		}

		/*! @brief Removes all AST::NoIds from the packed array of a sparse set. */
		void Compact()
		{
			size_type next = count;
			for (; next && ecs::GetVersion(packed[next - 1u]) == ecs::GetVersion(AST::NoId); --next)
			{}

			for (auto* it = &freeList; *it != AST::NoId && next;
			     it       = std::addressof(packed[ecs::GetIndex(*it)]))
			{
				if (const size_type pos = ecs::GetIndex(*it); pos < next)
				{
					--next;
					MoveAndPop(next, pos);
					std::swap(packed[next], packed[pos]);
					sparse[Page(packed[pos])][Offset(packed[pos])] =
					    AST::MakeId(static_cast<const typename traits_type::Entity>(pos));
					*it = AST::MakeId(static_cast<typename traits_type::Entity>(next));
					for (; next && ecs::GetVersion(packed[next - 1u]) == ecs::GetVersion(AST::NoId);
					     --next)
					{}
				}
			}

			freeList = AST::NoId;
			count    = next;
		}

		/**
		 * @copybrief swap_at
		 *
		 * For what it's worth, this function affects both the internal sparse array
		 * and the internal packed array. Users should not care of that anyway.
		 *
		 * @warning
		 * Attempting to swap entities that don't belong to the sparse set results
		 * in undefined behavior.
		 *
		 * @param lhs A valid entity identifier.
		 * @param rhs A valid entity identifier.
		 */
		void Swap(const entity_type lhs, const entity_type rhs)
		{
			CheckMsg(Contains(lhs), "Set does not contain entity");
			CheckMsg(Contains(rhs), "Set does not contain entity");

			auto& entt  = sparse[Page(lhs)][Offset(lhs)];
			auto& other = sparse[Page(rhs)][Offset(rhs)];

			const auto from = static_cast<size_type>(ecs::GetIndex(entt));
			const auto to   = static_cast<size_type>(ecs::GetIndex(other));

			// basic no-leak guarantee (with invalid state) if swapping throws
			SwapAt(from, to);
			std::swap(entt, other);
			std::swap(packed[from], packed[to]);
		}

		/**
		 * @brief Sort the first count elements according to the given comparison
		 * function.
		 *
		 * The comparison function object must return `true` if the first element
		 * is _less_ than the second one, `false` otherwise. The signature of the
		 * comparison function should be equivalent to the following:
		 *
		 * @code{.cpp}
		 * bool(const AST::Id, const AST::Id);
		 * @endcode
		 *
		 * Moreover, the comparison function object shall induce a
		 * _strict weak ordering_ on the values.
		 *
		 * The sort function object must offer a member function template
		 * `operator()` that accepts three arguments:
		 *
		 * * An iterator to the first element of the range to sort.
		 * * An iterator past the last element of the range to sort.
		 * * A comparison function to use to compare the elements.
		 *
		 * @tparam Compare Type of comparison function object.
		 * @tparam Sort Type of sort function object.
		 * @tparam Args Types of arguments to forward to the sort function object.
		 * @param length Number of elements to sort.
		 * @param compare A valid comparison function object.
		 * @param args Arguments to forward to the sort function object, if any.
		 */
		template<typename Compare, typename... Args>
		void SortN(const size_type length, Compare compare, Args&&... args)
		{
			// basic no-leak guarantee (with invalid state) if sorting throws
			CheckMsg(!(length > count), "Length exceeds the number of elements");
			Compact();

			std::sort(std::forward<Args>(args)..., std::make_reverse_iterator(packed + length),
			    std::make_reverse_iterator(packed), std::move(compare));

			for (size_type pos{}; pos < length; ++pos)
			{
				auto curr = pos;
				auto next = Index(packed[curr]);

				while (curr != next)
				{
					const auto idx  = Index(packed[next]);
					const auto entt = packed[curr];

					SwapAt(next, idx);
					sparse[Page(entt)][Offset(entt)] =
					    AST::MakeId(static_cast<typename traits_type::Entity>(curr));
					curr = std::exchange(next, idx);
				}
			}
		}

		/**
		 * @brief Sort all elements according to the given comparison function.
		 *
		 * @sa sort_n
		 *
		 * @tparam Compare Type of comparison function object.
		 * @tparam Sort Type of sort function object.
		 * @tparam Args Types of arguments to forward to the sort function object.
		 * @param compare A valid comparison function object.
		 * @param algo A valid sort function object.
		 * @param args Arguments to forward to the sort function object, if any.
		 */
		template<typename Compare, typename... Args>
		void Sort(Compare compare, Args&&... args)
		{
			SortN(count, std::move(compare), std::forward<Args>(args)...);
		}

		/**
		 * @brief Sort entities according to their order in another sparse set.
		 *
		 * Entities that are part of both the sparse sets are ordered internally
		 * according to the order they have in `other`. All the other entities goes
		 * to the end of the list and there are no guarantees on their order.<br/>
		 * In other terms, this function can be used to impose the same order on two
		 * sets by using one of them as a master and the other one as a slave.
		 *
		 * Iterating the sparse set with a couple of iterators returns elements in
		 * the expected order after a call to `respect`. See `begin` and `end` for
		 * more details.
		 *
		 * @param other The sparse sets that imposes the order of the entities.
		 */
		void Respect(const BasicSparseSet& other)
		{
			Compact();

			const auto to = other.end();
			auto from     = other.begin();

			for (size_type pos = count - 1; pos && from != to; ++from)
			{
				if (Contains(*from))
				{
					if (*from != packed[pos])
					{
						// basic no-leak guarantee (with invalid state) if swapping throws
						Swap(packed[pos], *from);
					}

					--pos;
				}
			}
		}

		/**
		 * @brief Clears a sparse set.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		void Clear(void* ud = nullptr)
		{
			for (auto&& entity : *this)
			{
				if (ecs::GetVersion(entity) != ecs::GetVersion(AST::NoId))
				{
					InPlacePop(entity, ud);
				}
			}

			freeList = AST::NoId;
			count    = 0u;
		}

	private:
		typename alloc_traits::allocator_type allocator;
		typename bucket_alloc_traits::allocator_type bucketAllocator;
		bucket_alloc_pointer sparse;
		alloc_pointer packed;
		std::size_t bucket;
		std::size_t count;
		std::size_t reserved;
		entity_type freeList;
		DeletionPolicy mode;
	};


}    // namespace rift::AST
