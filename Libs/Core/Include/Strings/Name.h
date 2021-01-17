// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Misc/Hash.h"
#include "Reflection/ClassTraits.h"
#include "String.h"

#include <EASTL/functional.h>
#include <EASTL/unordered_set.h>
#include <tsl/robin_set.h>

#include <mutex>
#include <shared_mutex>


namespace Rift
{
	struct Name;

	/** Represents an string with an already hashed value */
	struct NameKey
	{
	private:
		static const Hash<String> hasher;

		String str;
		size_t hash;

	public:
		NameKey(size_t hash = 0) : hash{hash} {}
		NameKey(StringView inStr) : str{inStr}, hash{hasher(str)} {}

		NameKey(const NameKey& other) : hash{other.hash} {}
		NameKey(NameKey&& other) : str{MoveTemp(other.str)}, hash{other.hash} {}
		NameKey& operator=(const NameKey& other)
		{
			hash = other.hash;
			return *this;
		}

		const String& GetString() const
		{
			return str;
		}
		const size_t GetHash() const
		{
			return hash;
		}

		bool operator==(const NameKey& other) const
		{
			return hash == other.hash;
		}
	};

	template <>
	struct Hash<NameKey>
	{
		size_t operator()(const NameKey& x) const
		{
			return x.GetHash();
		}
	};


	/** Global table storing all names */
	class NameTable
	{
		friend Name;

		// #TODO: Move to TSet
		using Container = tsl::robin_set<NameKey, Hash<NameKey>, eastl::equal_to<NameKey>>;
		using Iterator = Container::iterator;
		using ConstIterator = Container::const_iterator;

		Container table;
		// Mutex that allows sync reads but waits for registries
		mutable std::shared_mutex editTableMutex;


		NameTable() : table{} {}

		size_t Register(StringView string);
		const String& Find(size_t hash) const
		{
			// Ensure no other thread is editing the table
			std::shared_lock lock{editTableMutex};
			return table.find({hash})->GetString();
		}

		static NameTable& Get()
		{
			static NameTable instance{};
			return instance;
		}
	};


	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct Name
	{
		friend NameTable;
		using Id = size_t;

	private:
		static const String noneStr;
		Id id;


	public:
		Name() : id{noneId} {}
		Name(const TCHAR* key) : Name(StringView{key}) {}
		Name(StringView key)
		{
			// Index this name
			id = NameTable::Get().Register(key);
		}
		Name(const String& str) : Name(StringView(str)) {}
		Name(const Name& other) : id(other.id) {}
		Name(Name&& other) noexcept
		{
			std::swap(id, other.id);
		}
		Name& operator=(const Name& other)
		{
			id = other.id;
			return *this;
		}
		Name& operator=(Name&& other)
		{
			std::swap(id, other.id);
			return *this;
		}

		const String& ToString() const
		{
			return IsNone() ? noneStr : NameTable::Get().Find(id);
		}

		bool operator==(const Name& other) const
		{
			return id == other.id;
		}

		bool IsNone() const
		{
			return id == noneId;
		}

		const Id& GetId() const
		{
			return id;
		}


		static const Name None()
		{
			static Name none{noneId};
			return none;
		};
		static const Id noneId;

		bool Serialize(class Archive& ar, const char* name);

	private:
		Name(const Id& id) : id(id) {}
	};

	DEFINE_CLASS_TRAITS(Name, HasCustomSerialize = true);

	template <>
	struct Hash<Name>
	{
		size_t operator()(const Name& k) const
		{
			return k.GetId();
		}
	};
}	 // namespace Rift


namespace eastl
{
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
	EA_DISABLE_VC_WARNING(4455)	   // disable warning C4455: literal suffix identifiers that do
								   // not start with an underscore are reserved
	inline namespace literals
	{
		inline namespace String_literals
		{
			inline Rift::Name operator"" n(const Rift::TCHAR* str) EA_NOEXCEPT
			{
				return Rift::Name{str};
			}
		}					   // namespace String_literals
	}						   // namespace literals
	EA_RESTORE_VC_WARNING()	   // warning: 4455
#endif
}	 // namespace eastl
