// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include <mutex>
#include <EASTL/functional.h>
#include <EASTL/unordered_set.h>
#include <sparsehash/dense_hash_set.h>

#include "String.h"
#include "Core/Reflection/ClassTraits.h"
#include <shared_mutex>


struct Name;

/** Represents an string with an already hashed value */
struct NameKey
{
private:
	size_t hash;
	const String str;

public:
	NameKey(size_t hash = 0) : hash{ hash } {}
	NameKey(String str) : hash{ eastl::hash<String>()(str) }, str{ MoveTemp(str) } {}

	NameKey(const NameKey& other) : hash{ other.hash } {}
	NameKey(NameKey&& other) : hash{ other.hash }, str{MoveTemp(other.str)} {}
	NameKey& operator=(const NameKey& other) { hash = other.hash; return *this; }

	const String& GetString() const { return str;  }
	const size_t GetHash()    const { return hash; }

	bool operator==(const NameKey& other) const { return hash == other.hash; }
};

namespace eastl {
	template <>
	struct hash<NameKey>
	{
		FORCEINLINE size_t operator()(const NameKey& x) const { return x.GetHash(); }
	};
}


/** Global table storing all names */
class NameTable {
	friend Name;

	// #TODO: Move to TSet
	using Container     = google::dense_hash_set<NameKey, eastl::hash<NameKey>, eastl::equal_to<NameKey>>;
	using Iterator      = Container::iterator;
	using ConstIterator = Container::const_iterator;

	Container table;
	// Mutex that allows sync reads but waits for registries
	mutable std::shared_mutex editTableMutex;


	NameTable() : table{} {
		table.set_empty_key({ 0 });
	}

	size_t Register(const String& string);
	const String& Find(size_t hash) const
	{
		// Ensure no other thread is editing the table
		std::shared_lock lock{ editTableMutex };
		return table.find({ hash })->GetString();
	}

	static FORCEINLINE NameTable& GetGlobal() {
		static NameTable global {};
		return global;
	}
};


/**
 * An string identified by id.
 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more expensive.
 */
struct Name {
	friend NameTable;
	using Id = size_t;
private:

	static const String noneStr;
	Id id;


public:

	Name() : id{ noneId } {}

	Name(const StringView&& key) : Name(String{ key }) {}
	Name(const TCHAR* key) : Name(String{ key }) {}
	Name(const TCHAR* key, String::size_type size) : Name(String{ key, size }) {}
	Name(const String& key) {
		// Index this name
		id = NameTable::GetGlobal().Register(key);
	}
	Name(const Name& other) : id(other.id) {}
	Name(Name&& other) { std::swap(id, other.id); }

	Name& operator= (const Name& other) {
		id = other.id;
		return *this;
	}
	Name& operator= (Name&& other) {
		std::swap(id, other.id);
		return *this;
	}

	const String& ToString() const {
		return IsNone() ? noneStr : NameTable::GetGlobal().Find(id);
	}

	bool operator==(const Name& other) const {
		return id == other.id;
	}

	bool IsNone() const { return id == noneId; }

	const Id& GetId() const { return id; }


	static const Name None() {
		static Name none{ noneId };
		return none;
	};
	static const Id noneId;

	bool Serialize(class Archive& ar, const char* name);

private:

	Name(const Id& id) : id(id) {}
};

DEFINE_CLASS_TRAITS(Name,
	HasCustomSerialize = true
);

namespace eastl {
	template <>
	struct hash<Name> {
		size_t operator()(const Name& k) const
		{
			return k.GetId();
		}
	};

	/// user defined literals
	///
	/// Converts a character array literal to a basic_string.
	///
	/// Example:
	///   Name s = "abcdef"n;
	///
	/// http://en.cppreference.com/w/cpp/String/basic_String/operator%22%22s
	///
#if EASTL_USER_LITERALS_ENABLED && EASTL_INLINE_NAMESPACES_ENABLED
	EA_DISABLE_VC_WARNING(4455) // disable warning C4455: literal suffix identifiers that do not start with an underscore are reserved
	inline namespace literals
	{
		inline namespace String_literals
		{
			inline Name   operator"" n(const TCHAR* str, size_t len) EA_NOEXCEPT { return Name{ str, String::size_type(len) }; }
		}
	}
	EA_RESTORE_VC_WARNING()  // warning: 4455
#endif
} // namespace eastl
