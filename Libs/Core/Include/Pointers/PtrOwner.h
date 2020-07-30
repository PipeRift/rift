// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Platform/Platform.h"
#include "TypeTraits.h"

#include <atomic>


namespace VCLang
{
	namespace Impl
	{
		template <typename T>
		struct PtrBuilder
		{
			template <typename... Args>
			static T* New(Args&&... args)
			{
				return new T(std::forward<Args>(args)...);
			}

			static T* NewArray(size_t size)
			{
				using Elem = std::remove_extent_t<T>;
				return new Elem[size]();
			}

			static void Delete(void* ptr)
			{
				delete static_cast<T*>(ptr);
			}
		};

		// Container that lives from when an owner is created to when the last weak has been reset
		struct PtrWeakCounter
		{
			// Owner and weak references
			std::atomic<bool> bIsSet = true;
			std::atomic<u32> weaks = 0;
		};

		struct Ptr;

		struct PtrOwner
		{
			friend Ptr;

		protected:
			void* value = nullptr;
			PtrWeakCounter* counter = nullptr;


		public:
			bool IsValid() const
			{
				return counter != nullptr;
			}

			operator bool() const
			{
				return IsValid();
			};

		protected:
			PtrOwner() = default;
			PtrOwner(void* value) : value{value}
			{
				if (value)
				{
					counter = new PtrWeakCounter();
				}
			}

			void MoveFrom(PtrOwner&& other)
			{
				value = other.value;
				counter = other.counter;
				other.value = nullptr;
				other.counter = nullptr;
			}
		};


		struct Ptr
		{
		protected:
			void* value = nullptr;
			PtrWeakCounter* counter = nullptr;


		public:
			~Ptr()
			{
				Reset();
			}

			void Reset()
			{
				if (counter)
				{
					value = nullptr;

					if (--counter->weaks <= 0 && !counter->bIsSet)
					{
						delete counter;
					}
					counter = nullptr;
				}
			}

			bool IsValid() const
			{
				return counter && counter->bIsSet;
			}

			operator bool() const
			{
				return IsValid();
			};

		protected:
			Ptr() = default;
			Ptr(const PtrOwner& owner)
			{
				value = owner.value;
				counter = owner.counter;
				++counter->weaks;
			}
			Ptr(const Ptr& other)
			{
				value = other.value;
				counter = other.counter;
				++counter->weaks;
			}
			Ptr(Ptr&& other)
			{
				value = other.value;
				counter = other.counter;
				other.counter = nullptr;
			}

			void MoveFrom(Ptr&& other)
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

			void CopyFrom(const Ptr& other)
			{
				if (counter != other.counter)
				{
					Reset();
					value = other.value;
					counter = other.counter;
					++counter->weaks;
				}
			}
		};
	}	 // namespace Impl


	template <typename T>
	struct Ptr2;


	template <typename T, typename TBuilder = Impl::PtrBuilder<T>>
	struct PtrOwner : public Impl::PtrOwner
	{
		using Super = Impl::PtrOwner;

		template <typename T2, typename TBuilder2>
		friend struct PtrOwner;

	public:
		PtrOwner() = default;
		PtrOwner(T* value) : Super(value) {}

		PtrOwner(PtrOwner&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
		}

		template <typename T2>
		PtrOwner(PtrOwner<T2, TBuilder>&& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			MoveFrom(MoveTemp(other));
		}

		PtrOwner& operator=(PtrOwner&& other)
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}

		template <typename T2>
		PtrOwner& operator=(PtrOwner<T2, TBuilder>&& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			MoveFrom(MoveTemp(other));
			return *this;
		}

		~PtrOwner()
		{
			Release();
		}

		T* Get() const
		{
			return IsValid() ? operator*() : nullptr;
		}

		/** Cast a global pointer into another type. Will invalidate previous owner on success */
		template <typename T2>
		PtrOwner<T2, TBuilder> Cast()
		{
			static_assert(std::is_same_v<T2, T>, "Don't try to Cast using the same type");
			// If can be casted statically or dynamically
			if (IsValid() && (std::is_convertible_v<T2, T> || dynamic_cast<T2*>(**this) != nullptr))
			{
				PtrOwner<T2, TBuilder> newPtr{};
				newPtr.MoveFrom(MoveTemp(*this));
				return newPtr;
			}
			return {};
		}

		/** Cast a global pointer into another type. Will invalidate previous owner on success */
		template <typename T2>
		Ptr2<T2> Cast()
		{
			// If can be casted statically or dynamically
			if (IsValid())
			{
				Ptr2<T> ptr{*this};
				return ptr.template Cast<T2>();
			}
			return {};
		}

		void Release()
		{
			if (counter)
			{
				TBuilder::Delete(value);
				value = nullptr;

				counter->bIsSet = false;
				if (counter->weaks <= 0)
				{
					delete counter;
				}
				counter = nullptr;
			}
		}

		T* operator*() const
		{
			return static_cast<T*>(value);
		}
		T* operator->() const
		{
			return static_cast<T*>(value);
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return **this == other;
		}
		template <typename T2>
		bool operator==(const PtrOwner<T2, TBuilder>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator==(const Ptr2<T2>& other) const
		{
			return operator==(*other);
		}
	};


	template <typename T>
	struct Ptr2 : public Impl::Ptr
	{
		using Super = Impl::Ptr;

		Ptr2() = default;
		Ptr2(const Ptr2& other) : Super(other) {}
		Ptr2(Ptr2&& other) : Super(MoveTemp(other)) {}

		template <typename T2, typename TBuilder>
		Ptr2(const PtrOwner<T2, TBuilder>& owner) : Super(owner)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		template <typename T2>
		Ptr2(const Ptr2<T2>& other) : Super(other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		template <typename T2>
		Ptr2(Ptr2<T2>&& other) : Super(MoveTemp(other))
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		Ptr2& operator=(const Ptr2& other)
		{
			CopyFrom(other);
			return *this;
		}

		Ptr2& operator=(Ptr2&& other)
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}

		template <typename T2>
		Ptr2& operator=(const Ptr2<T2>& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			CopyFrom(other);
			return *this;
		}

		template <typename T2>
		Ptr2& operator=(Ptr2<T2>&& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			MoveFrom(MoveTemp(other));
			return *this;
		}

		T* operator*() const
		{
			return static_cast<T*>(value);
		}
		T* operator->() const
		{
			return static_cast<T*>(value);
		}

		T* Get() const
		{
			return IsValid() ? operator*() : nullptr;
		}

		template <typename T2>
		Ptr2<T2> Cast() const
		{
			if (IsValid() && (std::is_convertible_v<T2, T> || dynamic_cast<T2*>(**this) == nullptr))
			{
				Ptr2<T2> ptr{};
				ptr.CopyFrom(*this);
				return ptr;
			}
			return {};
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return **this == other;
		}
		template <typename T2, typename TBuilder>
		bool operator==(const PtrOwner<T2, TBuilder>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator==(const Ptr2<T2>& other) const
		{
			return operator==(*other);
		}
	};


	template <typename T, typename TBuilder = Impl::PtrBuilder<T>, typename... Args,
		EnableIfT<!std::is_array_v<T>, i32> = 0>
	PtrOwner<T, TBuilder> MakeOwned(Args&&... args)
	{
		return {TBuilder::New(std::forward<Args>(args)...)};
	}

	template <typename T, typename TBuilder = Impl::PtrBuilder<T>,
		EnableIfT<std::is_array_v<T> && std::extent_v<T> == 0, i32> = 0>
	PtrOwner<T, TBuilder> MakeOwned(size_t size)
	{
		using Elem = std::remove_extent_t<T>;
		return {TBuilder::NewArray(size)};
	}

	template <typename T, typename TBuilder = Impl::PtrBuilder<T>, typename... Args,
		EnableIfT<std::extent_v<T> != 0, i32> = 0>
	void MakeOwned(Args&&...) = delete;
}	 // namespace VCLang
