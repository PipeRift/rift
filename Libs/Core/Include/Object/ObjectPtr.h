// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "Log.h"
#include "Object/BaseObject.h"
#include "Platform/Platform.h"
#include "Reflection/ReflectionTypeTraits.h"

#include <memory>
#include <type_traits>


namespace VCLang
{
	class BaseWeakPtr;


	/** Non templated version of GlobalPtr that points to all weaks */
	class CORE_API BaseGlobalPtr
	{
		friend BaseWeakPtr;

		/** MEMBERS */
		mutable TArray<BaseWeakPtr*> weaks;

	protected:
		std::unique_ptr<BaseObject> ptr;


		/** METHODS */

		BaseGlobalPtr() : weaks{}, ptr{} {}
		BaseGlobalPtr(std::unique_ptr<BaseObject>&& inPtr) : BaseGlobalPtr()
		{
			ptr = MoveTemp(inPtr);
		}

		BaseGlobalPtr(BaseGlobalPtr&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
		}

		BaseGlobalPtr& operator=(BaseGlobalPtr&& other)
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}

		~BaseGlobalPtr();

		void MoveFrom(BaseGlobalPtr&& other);

	public:
		BaseObject* operator*() const
		{
			return ptr.get();
		}
		BaseObject* operator->() const
		{
			return ptr.get();
		}

		bool IsValid() const
		{
			return ptr.get() != nullptr;
		}
		operator bool() const
		{
			return IsValid();
		};

		void Reset()
		{
			if (IsValid())
			{
				ptr->StartDestroy();
				ptr.release();
			}
		}
	};

	template <typename T>
	class Ptr;


	/**
	 * Pointer to an Object that keeps it from being removed.
	 * If a GlobalPtr goes out of scope or destroyed the object will be too.
	 * GlobalPtrs are Unique. An object can't be referenced by more than one global ptr.
	 */
	template <typename T>
	class CORE_API GlobalPtr : public BaseGlobalPtr
	{
		static_assert(std::is_convertible<T, BaseObject>::value, "Type is not an Object!");

		template <typename T2>
		friend class GlobalPtr;


		/** METHODS */

		GlobalPtr(std::unique_ptr<T>&& inPtr) : BaseGlobalPtr()
		{
			ptr = std::move(inPtr);
		}

	public:
		GlobalPtr() = default;

		~GlobalPtr()
		{
			Reset();
		}

		GlobalPtr(GlobalPtr<T>&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
		}

		template <typename T2>
		GlobalPtr(GlobalPtr<T2>&& other)
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			MoveFrom(MoveTemp(other));
		}

		template <typename T2>
		GlobalPtr& operator=(GlobalPtr<T2>&& other)
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");

			MoveFrom(MoveTemp(other));
			return *this;
		}

		Ptr<T> AsPtr() const
		{
			return Ptr<T>(*this);
		}
		operator Ptr<T>() const
		{
			return AsPtr();
		}

		T* operator*() const
		{
			return static_cast<T*>(BaseGlobalPtr::operator*());
		}
		T* operator->() const
		{
			return static_cast<T*>(BaseGlobalPtr::operator->());
		}

		template <typename T2>
		bool operator==(const GlobalPtr<T2>& other) const
		{
			return **this == *other;
		}
		template <typename T2>
		bool operator==(const Ptr<T2>& other) const
		{
			return **this == *other;
		}

		/** Cast a global pointer into another type. Will invalidate previous target when success */
		template <typename T2>
		GlobalPtr<T2> Cast()
		{
			if (!IsValid() || dynamic_cast<T2*>(**this) == nullptr)
				return {};
			else
			{
				GlobalPtr<T2> newPtr{};
				newPtr.MoveFrom(MoveTemp(*this));
				return newPtr;
			}
		}

		/** INTERNAL USAGE ONLY. Use "Create<YourObject>" instead */
		static GlobalPtr<T> Create(const Ptr<BaseObject>& owner);
	};

	/** Non templated version of WeakPtr that points to a globalPtr */
	class CORE_API BaseWeakPtr
	{
		friend BaseGlobalPtr;

	private:
		// #TODO: Optimize storing pointer to value
		const BaseGlobalPtr* globalPtr;
		u32 id;

	protected:
		BaseWeakPtr() : globalPtr{nullptr}, id{0} {}

		~BaseWeakPtr()
		{
			UnBind();
		}

		void Set(const BaseGlobalPtr* inOwner);

		void MoveFrom(BaseWeakPtr&& other);

		const BaseGlobalPtr* GetGlobal() const
		{
			return globalPtr;
		}

	public:
		bool IsValid() const
		{
			return globalPtr != nullptr && globalPtr->IsValid();
		}

		void Reset()
		{
			UnBind();
			CleanOwner();
		}

	protected:
		void CleanOwner()
		{
			globalPtr = nullptr;
		}

		void UnBind()
		{
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
	 * Objects will be removed if their global ptr is destroyed. In this case all pointers will be
	 * invalidated.
	 */
	template <typename T>
	class CORE_API Ptr : public BaseWeakPtr
	{
		static_assert(std::is_convertible<T, BaseObject>::value, "Type is not an Object!");

		friend GlobalPtr<T>;
		template <typename T2>
		friend class Ptr;


		/** METHODS */

	public:
		Ptr() : BaseWeakPtr() {}
		Ptr(TYPE_OF_NULLPTR) : BaseWeakPtr() {}

		Ptr(const Ptr& other) : BaseWeakPtr()
		{
			Set(other.GetGlobal());
		}
		Ptr& operator=(const Ptr& other)
		{
			Set(other.GetGlobal());
			return *this;
		}

		Ptr(Ptr&& other) : BaseWeakPtr()
		{
			MoveFrom(MoveTemp(other));
		}
		Ptr& operator=(Ptr&& other)
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}

		/** Templates for down-casting */
		template <typename T2>
		Ptr(const Ptr<T2>& other) : BaseWeakPtr()
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			Set(other.GetGlobal());
		}
		template <typename T2>
		Ptr& operator=(const Ptr<T2>& other)
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			Set(other.GetGlobal());
			return *this;
		};

		template <typename T2>
		Ptr(Ptr<T2>&& other) : BaseWeakPtr()
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			MoveFrom(MoveTemp(other));
		}
		template <typename T2>
		Ptr& operator=(Ptr<T2>&& other)
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			MoveFrom(MoveTemp(other));
			return *this;
		}

		template <typename T2>
		Ptr(const GlobalPtr<T2>& other) : BaseWeakPtr()
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			Set(&other);
		}

		template <typename T2>
		Ptr(T2* other) : BaseWeakPtr()
		{
			static_assert(
				std::is_same_v<T2, T> || std::is_convertible_v<T2, T>, "Type is not compatible!");
			if (!other)
			{
				Reset();
				return;
			}
			Ptr<T2>(other->Self());
		}


		template <typename T2>
		Ptr& operator=(const GlobalPtr<T2>& other)
		{
			Set(&other);
			return *this;
		};

		Ptr& operator=(TYPE_OF_NULLPTR)
		{
			Reset();
			return *this;
		};


		template <typename T2>
		bool operator==(const Ptr<T2>& other) const
		{
			return GetGlobal() == other.GetGlobal();
		}

		template <typename T2>
		bool operator==(const GlobalPtr<T2>& other) const
		{
			return other == *this;
		}

		template <typename T2>
		bool operator!=(const Ptr<T2>& other) const
		{
			return !operator==(other);
		}

		template <typename T2>
		bool operator!=(const GlobalPtr<T2>& other) const
		{
			return !operator==(other);
		}

		T* operator*() const
		{
			// Static cast since types are always cast-able or invalid
			return IsValid() ? static_cast<T*>(**GetGlobal()) : nullptr;
		}
		T* operator->() const
		{
			// #OPTIMIZE: Jumping to GlobalPtr and then Value ptr can produce cache misses. Consider
			// accessing value directly. Static cast since types are always cast-able or invalid
			return IsValid() ? static_cast<T*>(**GetGlobal()) : nullptr;
		}

		operator bool() const
		{
			return IsValid();
		};

		template <typename T2>
		Ptr<T2> Cast() const
		{
			if (!IsValid() || dynamic_cast<T2*>(**this) == nullptr)
				return {};
			else
			{
				Ptr<T2> ptr{};
				ptr.Set(GetGlobal());
				return ptr;
			}
		}
	};


	template <typename T>
	GlobalPtr<T> GlobalPtr<T>::Create(const Ptr<BaseObject>& owner)
	{
		if constexpr (std::is_abstract_v<T>)
		{
			Log::Error("Tried to create an instance of '{}' which is abstract.",
				GetReflectableName<T>().ToString());
			return {};
		}
		else
		{
			GlobalPtr<T> ptr{std::make_unique<T>()};
			ptr->PreConstruct(ptr.AsPtr(), T::StaticType(), owner);
			ptr->Construct();
			return MoveTemp(ptr);
		}
	}
}	 // namespace VCLang
