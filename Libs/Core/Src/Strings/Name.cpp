// Copyright 2015-2020 Piperift - All rights reserved

#include "Strings/Name.h"

#include "Serialization/Archive.h"


namespace Rift
{
	const Hash<String> NameKey::hasher{};

	const String Name::noneStr{"none"};
	const Name::Id Name::noneId{0};

	size_t NameTable::Register(StringView str)
	{
		if (str.empty())
		{
			return Name::noneId;
		}

		// Calculate hash once
		NameKey key{str};
		ConstIterator FoundIt = table.find(key);
		if (FoundIt != table.end())
		{
			std::shared_lock lock{editTableMutex};
			return FoundIt->GetHash();
		}
		else
		{
			std::unique_lock lock{editTableMutex};
			return table.insert(MoveTemp(key)).first->GetHash();
		}
	}

	bool Name::Serialize(Archive& ar, const char* name)
	{
		if (ar.IsSaving())
		{
			String str = ToString();
			ar(name, str);
		}
		else
		{
			String str;
			ar(name, str);

			if (CString::Equals(str, noneStr))
			{
				*this = None();
			}
			else
			{
				*this = str;
			}
		}
		return true;
	}
}	 // namespace Rift
