// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <EASTL/weak_ptr.h>

#include "Core/Platform/Platform.h"
#include "Core/Object/BaseObject.h"


class BaseWeakPtr;


/** Non templated version of GlobalPtr that points to all weaks */
class BaseGlobalPtr {
	friend BaseWeakPtr;

	/** MEMBERS */
	mutable TArray<BaseWeakPtr*> weaks;

protected:

	eastl::unique_ptr<BaseObject> ptr;


	/** METHODS */

	BaseGlobalPtr() : weaks{}, ptr{} {}
	BaseGlobalPtr(eastl::unique_ptr<BaseObject>&& inPtr) : BaseGlobalPtr() {
		ptr = eastl::move(inPtr);
	}

	BaseGlobalPtr(BaseGlobalPtr&& other) {
		MoveFrom(eastl::move(other));
	}

	BaseGlobalPtr& operator=(BaseGlobalPtr&& other) {
		MoveFrom(eastl::move(other));
		return *this;
	}

	~BaseGlobalPtr();

	void MoveFrom(BaseGlobalPtr&& other);

public:

	BaseObject* operator*()  const { return ptr.get(); }
	BaseObject* operator->() const { return ptr.get(); }

	bool IsValid() const { return ptr.get() != nullptr; }
	operator bool() const { return IsValid(); };

	void Reset() {
		if (IsValid())
		{
			ptr->StartDestroy();
			ptr.release();
		}
	}
};

template<typename Type>
class Ptr;


/**
 * Pointer to an Object that keeps it from being removed.
 * If a GlobalPtr goes out of scope or destroyed the object will be too.
 * GlobalPtrs are Unique. An object can't be referenced by more than one global ptr.
 */
template<typename Type>
class GlobalPtr : public BaseGlobalPtr
{
	static_assert(eastl::is_convertible< Type, BaseObject >::value, "Type is not an Object!");

	friend class GlobalPtr;


	/** METHODS */

	GlobalPtr(eastl::unique_ptr<Type>&& inPtr) : BaseGlobalPtr() { ptr = std::move(inPtr); }

public:

	GlobalPtr() = default;

	~GlobalPtr() { Reset(); }


	template<typename Type2>
	GlobalPtr(GlobalPtr<Type2>&& other) { operator=<Type2>(eastl::move(other)); }

	template<typename Type2>
	GlobalPtr& operator=(GlobalPtr<Type2>&& other) {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");

		if (!other.IsValid())
			Reset();
		else
			BaseGlobalPtr::operator=(eastl::move(other));
		return *this;
	}

	Ptr<Type> AsPtr() const { return Ptr<Type>(*this); }
	operator Ptr<Type>() const { return AsPtr(); }

	Type* operator*()  const { return static_cast<Type*>(BaseGlobalPtr::operator*());  }
	Type* operator->() const { return static_cast<Type*>(BaseGlobalPtr::operator->()); }

	template<typename Type2>
	bool operator==(const GlobalPtr<Type2>& other) const { return **this == *other; }
	template<typename Type2>
	bool operator==(const Ptr<Type2>& other) const { return **this == *other; }

	/** Cast a global pointer into another type. Will invalidate previous target when success */
	template<typename T>
	GlobalPtr<T> Cast() {
		if (!IsValid() || dynamic_cast<Type*>(**this) == nullptr)
			return {};
		else
		{
			GlobalPtr<T> newPtr{};
			newPtr.MoveFrom(eastl::move(*this));
			return newPtr;
		}
	}

	/** INTERNAL USAGE ONLY. Use "Create<YourObject>" instead */
	static GlobalPtr<Type> Create(const Ptr<BaseObject>& owner);
};

/** Non templated version of WeakPtr that points to a globalPtr */
class BaseWeakPtr {
	friend BaseGlobalPtr;

	// #TODO: Optimize storing pointer to value
	const BaseGlobalPtr* globalPtr;
	u32 id;

protected:

	BaseWeakPtr() : globalPtr{ nullptr }, id{ 0 } {}

	~BaseWeakPtr() {
		UnBind();
	}

	void Set(const BaseGlobalPtr* inOwner);

	void MoveFrom(BaseWeakPtr&& other);

	const BaseGlobalPtr* GetGlobal() const { return globalPtr; }

public:

	BaseGlobalPtr* __GetGlobal() const { return const_cast<BaseGlobalPtr*>(globalPtr); }

	bool IsValid() const {
		return globalPtr != nullptr && globalPtr->IsValid();
	}

	void Reset() {
		UnBind();
		CleanOwner();
	}

protected:

	void CleanOwner() {
		globalPtr = nullptr;
	}

	void UnBind() {
		// Remove old weak ptr
		if (globalPtr != nullptr)
		{
			globalPtr->weaks.RemoveAt(id);

			// All weaks after this one have id - 1
			for (i32 i = id; i < globalPtr->weaks.Size(); ++i)
			{
				--globalPtr->weaks[i]->id;
			}
		}
	}
};


/**
* Weak Object Pointers
* Objects will be removed if their global ptr is destroyed. In this case all pointers will be invalidated.
*/
template<typename Type>
class Ptr : public BaseWeakPtr
{
	static_assert(eastl::is_convertible<Type, BaseObject>::value, "Type is not an Object!");

	friend GlobalPtr<Type>;
	friend class Ptr;


	/** METHODS */

public:

	Ptr() : BaseWeakPtr() {}
	Ptr(TYPE_OF_NULLPTR) : BaseWeakPtr() {}

	Ptr(const Ptr& other) : BaseWeakPtr() {
		Set(other.GetGlobal());
	}
	Ptr& operator=(const Ptr& other) {
		Set(other.GetGlobal());
		return *this;
	}

	Ptr(Ptr&& other) : BaseWeakPtr() {
		MoveFrom(eastl::move(other));
	}
	Ptr& operator=(Ptr&& other) {
		MoveFrom(eastl::move(other));
		return *this;
	}

	/** Templates for down-casting */
	template<typename Type2>
	Ptr(const Ptr<Type2>& other) : BaseWeakPtr() {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");
		Set(other.GetGlobal());
	}
	template<typename Type2>
	Ptr& operator=(const Ptr<Type2>& other) {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");
		Set(other.GetGlobal());
		return *this;
	};

	template<typename Type2>
	Ptr(Ptr<Type2>&& other) : BaseWeakPtr() {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");
		MoveFrom(eastl::move(other));
	}
	template<typename Type2>
	Ptr& operator=(Ptr<Type2>&& other) {
		static_assert(eastl::is_convertible<Type2, Type>::value, "Type is not compatible!");
		MoveFrom(eastl::move(other));
		return *this;
	}

	template<typename Type2>
	Ptr(const GlobalPtr<Type2>& other) : BaseWeakPtr() {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");
		Set(&other);
	}

	template<typename Type2>
	Ptr(Type2* other) : BaseWeakPtr() {
		static_assert(eastl::is_convertible< Type2, Type >::value, "Type is not compatible!");
		if (!other) {
			Reset();
			return;
		}
		Ptr<Type2>(other->GetSelf());
	}


	template<typename Type2>
	Ptr& operator=(const GlobalPtr<Type2>& other) {
		Set(&other);
		return *this;
	};

	Ptr& operator=(TYPE_OF_NULLPTR) {
		Reset();
		return *this;
	};


	template<typename Type2>
	bool operator==(const Ptr<Type2>& other) const {
		return GetGlobal() == other.GetGlobal();
	}

	template<typename Type2>
	bool operator==(const GlobalPtr<Type2>& other) const { return other == *this; }


	template<typename Type2>
	bool operator!=(const Ptr<Type2>& other) const { return !operator==(other); }

	template<typename Type2>
	bool operator!=(const GlobalPtr<Type2>& other) const { return !operator==(other); }


	Type* operator*() const {
		// Static cast since types are always cast-able or invalid
		return IsValid() ? static_cast<Type*>(GetGlobal()->operator*()) : nullptr;
	}
	Type* operator->() const {
		// #OPTIMIZE: Jumping to GlobalPtr and then Value ptr can produce cache misses. Consider accessing value directly.
		// Static cast since types are always cast-able or invalid
		return IsValid() ? static_cast<Type*>(GetGlobal()->operator->()) : nullptr;
	}

	operator bool() const { return IsValid(); };

	template<typename T>
	Ptr<T> Cast() const {
		if (!IsValid() || dynamic_cast<Type*>(**this) == nullptr)
			return {};
		else
		{
			Ptr<T> ptr{};
			ptr.Set(GetGlobal());
			return ptr;
		}
	}
};


template<typename Type>
GlobalPtr<Type> GlobalPtr<Type>::Create(const Ptr<BaseObject>& owner)
{
	GlobalPtr<Type> ptr = { eastl::make_unique<Type>() };
	ptr->PreConstruct(ptr.AsPtr(), Type::StaticClass(), owner);
	ptr->Construct();
	return eastl::move(ptr);
}
