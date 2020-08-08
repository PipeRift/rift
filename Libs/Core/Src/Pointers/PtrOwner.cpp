// Copyright 2015-2020 Piperift - All rights reserved

#include "Pointers/PtrOwner.h"


namespace VCLang::Impl
{
	void Ptr::Reset()
	{
		if (counter)
		{
			__ResetNoCheck(counter->bIsSet);
		}
	}

	bool Ptr::IsValid() const
	{
		if (counter)
		{
			if (counter->bIsSet)
			{
				return true;
			}
			const_cast<Ptr*>(this)->__ResetNoCheck(false);
		}
		return false;
	}

	Ptr::Ptr(const PtrOwner& owner)
	{
		value = owner.value;
		counter = owner.counter;
		if (counter)
		{
			++counter->weaks;
		}
	}
	Ptr::Ptr(const Ptr& other)
	{
		value = other.value;
		counter = other.counter;
		if (counter)
		{
			++counter->weaks;
		}
	}
	Ptr::Ptr(Ptr&& other)
	{
		value = other.value;
		counter = other.counter;
		other.counter = nullptr;
	}

	void Ptr::MoveFrom(Ptr&& other)
	{
		if (counter != other.counter)
		{
			Reset();
			value = other.value;
			counter = other.counter;
			other.counter = nullptr;
		}
		else	// If equals, we reset previous anyway
		{
			other.Reset();
		}
	}

	void Ptr::CopyFrom(const Ptr& other)
	{
		if (counter != other.counter)
		{
			Reset();
			value = other.value;
			counter = other.counter;
			if (counter)
			{
				++counter->weaks;
			}
		}
	}

	void Ptr::__ResetNoCheck(const bool bIsSet)
	{
		if (--counter->weaks <= 0 && !bIsSet)
		{
			delete counter;
		}
		counter = nullptr;
		value = nullptr;
	}
}	 // namespace VCLang::Impl
