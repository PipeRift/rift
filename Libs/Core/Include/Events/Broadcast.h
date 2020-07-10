// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "CoreTypes.h"
#include "EventHandle.h"
#include "Log.h"


template <typename... Params>
class Broadcast
{
protected:
	using Method = void(Params...);
	using Function = eastl::function<void(Params...)>;

	using MethodPtr = void (*)(Params...);
	template <typename Type>
	using MemberMethodPtr = void (Type::*)(Params...);


	struct BaseListener
	{
		u64 id;
		Function method;
	};
	struct RawListener : public BaseListener
	{
		void* instance;
	};
	struct ObjListener : public BaseListener
	{
		Ptr<Object> object;
	};

	mutable TArray<RawListener> rawListeners;
	mutable TArray<ObjListener> objListeners;


public:
	Broadcast() : rawListeners{}, objListeners{} {}

	/** Broadcast to all binded functions */
	void DoBroadcast(const Params&... params)
	{
		for (RawListener& listener : rawListeners)
		{
			listener.method(params...);
		}

		for (auto& listener : objListeners)
		{
			if (listener.object)
			{
				listener.method(params...);
			}
		}
	}

	// #TODO: Scoped binding

	/** Binds an static function. Must be unbinded manually. */
	EventHandle Bind(Function method) const
	{
		if (method)
		{
			EventHandle handle{};
			rawListeners.Add({handle.Id(), MoveTemp(method), nullptr});
			return handle;
		}

		Log::Warning("Couldn't bind delegate");
		return EventHandle::Invalid();
	}

	/** Binds a member function. Must be unbinded manually (unless its an
	 * Object). */
	template <typename Type>
	EventHandle Bind(Type* instance, MemberMethodPtr<Type> method) const
	{
		if (instance && method)
		{
			if constexpr (IsObject<Type>::value)
			{
				return Bind<Type>(instance->Self(), MoveTemp(method));
			}
			else
			{
				return Bind([instance, method](Params... params) {
					(instance->*method)(params...);
				});
			}
		}

		Log::Warning("Couldn't bind delegate");
		return EventHandle::Invalid();
	}

	/** Binds an object's function. Gets unbinded when the objects is destroyed
	 */
	template <typename Type>
	EventHandle Bind(Ptr<Type> object, MemberMethodPtr<Type> method) const
	{
		if (object && method)
		{
			Type* const instance = *object;
			Function func = [instance, method](Params... params) {
				(instance->*method)(params...);
			};

			EventHandle handle{};
			objListeners.Add({handle.Id(), MoveTemp(func), object});
			return handle;
		}

		Log::Warning("Couldn't bind delegate");
		return EventHandle::Invalid();
	}

	bool Unbind(const EventHandle& handle) const
	{
		if (!handle)
			return false;

		return rawListeners.RemoveIf([handle](const auto& listener) {
			return listener.id == handle.Id();
		}) > 0;
	}

	bool UnbindAll(Ptr<Object> object) const
	{
		if (object)
		{
			return objListeners.RemoveIf([object](const auto& listener) {
				return !listener.object || listener.object == object;
			}) > 0;
		}
		return false;
	}

	template <typename Type>
	bool UnbindAll(Type* instance) const
	{
		if (instance)
		{
			if constexpr (IsObject<Type>::value)
			{
				return UnbindAll(instance->Self());
			}
			else
			{
				return rawListeners.RemoveIf([instance](const auto& listener) {
					return listener.instance == instance;
				}) > 0;
			}
		}
		return false;
	}
};
