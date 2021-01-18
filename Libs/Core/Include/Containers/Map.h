// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Containers/Tuples.h"
#include "CoreEngine.h"
#include "Misc/Hash.h"
#include "Platform/Platform.h"

#include <assert.h>
#include <tsl/sparse_map.h>

#include <type_traits>


namespace Rift
{
	template <typename Key, typename Value>
	class TMap
	{
		static_assert(std::is_nothrow_move_constructible<Value>::value ||
						  std::is_copy_constructible<Value>::value,
			"Value type must be nothrow move constructible and/or copy constructible.");

	public:
		template <typename OtherKey, typename OtherValue>
		friend class TMap;

		using KeyType = Key;
		using ValueType = Value;
		using HashMapType =
			tsl::sparse_map<KeyType, ValueType, Hash<KeyType>, std::equal_to<KeyType>>;

		using Iterator = typename HashMapType::iterator;
		using ConstIterator = typename HashMapType::const_iterator;


	private:
		HashMapType map;


	public:
		TMap() = default;
		TMap(u32 defaultSize) : map{defaultSize} {}
		TMap(const TPair<const KeyType, ValueType>& item) : map{}
		{
			Insert(item);
		}
		TMap(std::initializer_list<TPair<const KeyType, ValueType>> initList)
			: map{initList.begin(), initList.end()}
		{}

		TMap(TMap&& other) = default;
		TMap(const TMap& other) = default;
		TMap& operator=(TMap&& other) = default;
		TMap& operator=(const TMap& other) = default;

		void Insert(KeyType&& key, ValueType&& value)
		{
			map.insert({MoveTemp(key), MoveTemp(value)});
		}

		void Insert(const KeyType& key, ValueType&& value)
		{
			map.insert({key, MoveTemp(value)});
		}

		void Insert(KeyType&& key, const ValueType& value)
		{
			map.insert({MoveTemp(key), value});
		}

		void Insert(const KeyType& key, const ValueType& value)
		{
			map.insert({key, value});
		}

		void Insert(const TPair<KeyType, ValueType>& pair)
		{
			map.insert(pair);
		}

		void Append(const TMap<KeyType, ValueType>& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					CopyFrom(other);
				}
				else
				{
					map.insert(other.begin(), other.end());
				}
			}
		}

		void Append(TMap<KeyType, ValueType>&& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					MoveFrom(MoveTemp(other));
				}
				else
				{
					map.insert(map.end(), other.begin(), other.end());
				}
			}
		}

		void Resize(i32 sizeNum)
		{
			// map.resize(sizeNum);
		}

		Iterator FindIt(const KeyType& item)
		{
			return map.find(item);
		}

		ConstIterator FindIt(const KeyType& item) const
		{
			return map.find(item);
		}

		ValueType* Find(const KeyType& key)
		{
			Iterator it = FindIt(key);
			return it != end() ? &it->second : nullptr;
		}

		const ValueType* Find(const KeyType& key) const
		{
			ConstIterator it = FindIt(key);
			return it != end() ? &it->second : nullptr;
		}

		ValueType& FindRef(const KeyType& key)
		{
			ConstIterator it = FindIt(key);
			assert(it != end() && "Key not found, can't dereference its value");
			return it->second;
		}

		const ValueType& FindRef(const KeyType& key) const
		{
			ConstIterator it = FindIt(key);
			assert(it != end() && "Key not found, can't dereference its value");
			return it->second;
		}

		bool Contains(const KeyType& key) const
		{
			return FindIt(key) != map.end();
		}

		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		i32 Remove(const KeyType& key)
		{
			Iterator it = FindIt(key);
			if (it != end())
			{
				const i32 lastSize = Size();
				map.erase(it);
				return lastSize - Size();
			}
			return 0;
		}

		/** Empty the array.
		 * @param bShouldShrink false will not free memory
		 */
		void Empty(const bool bShouldShrink = true, i32 sizeNum = 0)
		{
			if (bShouldShrink)
			{
				map.clear();
			}
			else
			{
				map.clear_no_resize();
				if (sizeNum > 0 && map.max_size() != sizeNum)
				{
					map.resize(sizeNum);
				}
			}
		}

		i32 Size() const
		{
			return (i32) map.size();
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}


		/** OPERATORS */
	public:
		/**
		 * Array bracket operator. Returns reference to value at given key.
		 *
		 * @returns Reference to indexed element.
		 */
		ValueType& operator[](const KeyType& key)
		{
			return map[key];
		}
		const ValueType& operator[](const KeyType& key) const
		{
			return map[key];
		}
		ValueType& operator[](KeyType&& key)
		{
			return map[MoveTemp(key)];
		}
		const ValueType& operator[](KeyType&& key) const
		{
			return map[MoveTemp(key)];
		}

		// Iterator functions
		Iterator begin()
		{
			return map.begin();
		};
		ConstIterator begin() const
		{
			return map.begin();
		};
		ConstIterator cbegin() const
		{
			return map.cbegin();
		};

		Iterator end()
		{
			return map.end();
		};
		ConstIterator end() const
		{
			return map.end();
		};
		ConstIterator cend() const
		{
			return map.cend();
		};


		/** INTERNAL */
	private:
		void CopyFrom(const TMap<KeyType, ValueType>& other)
		{
			map = other.map;
		}

		void MoveFrom(TMap<KeyType, ValueType>&& other)
		{
			map = MoveTemp(other.map);
		}
	};
}	 // namespace Rift
