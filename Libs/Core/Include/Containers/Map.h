// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include <assert.h>
#include <sparsehash/sparse_hash_map.h>
#include <sparsehash/dense_hash_map.h>
#include <EASTL/functional.h>

#include "CoreEngine.h"
#include "Core/Platform/Platform.h"
#include "Tuples.h"


template<typename Key, typename Value>
class TMap {
public:
	template <typename OtherKey, typename OtherValue>
	friend class TMap;

	using KeyType = Key;
	using ValueType = Value;
	using HashMapType = google::dense_hash_map<
		KeyType,
		ValueType,
		eastl::hash<KeyType>,
		eastl::equal_to<KeyType>,
		google::libc_allocator_with_realloc<TPair<const KeyType, ValueType>>
	>;

	using Iterator           = typename HashMapType::iterator;
	using ConstIterator      = typename HashMapType::const_iterator;


private:

	HashMapType map;


public:

	TMap() = default;

	TMap(const KeyType& empty) : map{} { SetEmptyKey(empty); }
	TMap(u32 defaultSize) : map{ defaultSize } {}
	TMap(u32 defaultSize, const KeyType& empty) : map{ defaultSize } { SetEmptyKey(empty); }
	TMap(const TPair<const KeyType, ValueType>& item, const KeyType& empty) : map{} { SetEmptyKey(empty); Insert(item); }
	TMap(std::initializer_list<TPair<const KeyType, ValueType>> initList, const KeyType& empty) : map{ initList.begin(), initList.end(), empty } {}

	TMap(TMap&& other) = default;
	TMap(const TMap& other) = default;
	TMap& operator=(TMap&& other) = default;
	TMap& operator=(const TMap& other) = default;

	void Insert(KeyType&& key, ValueType&& value) {
		map.insert({ MoveTemp(key), MoveTemp(value) });
	}

	void Insert(const KeyType& key, ValueType&& value) {
		map.insert({ key, MoveTemp(value) });
	}

	void Insert(KeyType&& key, const ValueType& value) {
		map.insert({ MoveTemp(key), value });
	}

	void Insert(const KeyType& key, const ValueType& value) {
		map.insert({ key, value });
	}

	void Insert(const TPair<KeyType, ValueType>& pair) {
		map.insert(pair);
	}

	void Append(const TMap<KeyType, ValueType>& other) {
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

	void Append(TMap<KeyType, ValueType>&& other) {
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

	void Resize (i32 sizeNum) { map.resize(sizeNum); }

	FORCEINLINE Iterator FindIt(const KeyType& item) {
		return map.find(item);
	}

	FORCEINLINE ConstIterator FindIt(const KeyType& item) const {
		return map.find(item);
	}

	FORCEINLINE ValueType* Find(const KeyType& key) {
		Iterator it = FindIt(key);
		return it != end() ? &it->second : nullptr;
	}

	FORCEINLINE const ValueType* Find(const KeyType& key) const {
		ConstIterator it = FindIt(key);
		return it != end() ? &it->second : nullptr;
	}

	FORCEINLINE ValueType& FindRef(const KeyType& key) {
		ConstIterator it = FindIt(key);
		assert(it != end() && "Key not found, can't dereference its value");
		return it->second;
	}

	FORCEINLINE const ValueType& FindRef(const KeyType& key) const {
		ConstIterator it = FindIt(key);
		assert(it != end() && "Key not found, can't dereference its value");
		return it->second;
	}

	bool Contains(const KeyType& key) const {
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
	void Empty(const bool bShouldShrink = true, i32 sizeNum = 0) {

		if(bShouldShrink)
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

	FORCEINLINE i32 Size() const { return (i32)map.size(); }

	FORCEINLINE bool IsValidIndex(i32 index) const
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
	FORCEINLINE ValueType& operator[](const KeyType& key) { return map[key]; }
	FORCEINLINE const ValueType& operator[](const KeyType& key) const {
		return map[key];
	}
	FORCEINLINE ValueType& operator[](KeyType&& key) { return map[MoveTemp(key)]; }
	FORCEINLINE const ValueType& operator[](KeyType&& key) const {
		return map[MoveTemp(key)];
	}

	void SetEmptyKey(const KeyType& key) {
		map.set_empty_key(key);
	}

	void SetDeletedKey(const KeyType& key) {
		map.set_deleted_key(key);
	}

	// Iterator functions
	FORCEINLINE Iterator      begin()        { return map.begin();  };
	FORCEINLINE ConstIterator begin()  const { return map.begin();  };
	FORCEINLINE ConstIterator cbegin() const { return map.cbegin(); };

	FORCEINLINE Iterator      end()        { return map.end();  };
	FORCEINLINE ConstIterator end()  const { return map.end();  };
	FORCEINLINE ConstIterator cend() const { return map.cend(); };


	/** INTERNAL */
private:

	FORCEINLINE void CopyFrom(const TMap<KeyType, ValueType>& other) {
		map = other.map;
	}

	FORCEINLINE void MoveFrom(TMap<KeyType, ValueType>&& other) {
		map = MoveTemp(other.map);
	}
};
