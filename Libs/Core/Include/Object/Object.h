// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "BaseObject.h"
#include "Log.h"
#include "ObjectBuilder.h"
#include "Reflection/Class.h"
#include "Serialization/Archive.h"


namespace Rift
{
	class Context;

	class CORE_API Object : public BaseObject
	{
		ORPHAN_CLASS(Object, ReflectionTags::None)

	private:
		PROP(Name, name);
		Name name;

		Refl::Class* ownClass;
		Ptr<BaseObject> self;
		Ptr<BaseObject> owner;


	public:
		Object() : BaseObject(), ownClass{nullptr}, self{}, owner{} {};

		void PreConstruct(
			Ptr<BaseObject>&& inSelf, Refl::Class* inClass, const Ptr<BaseObject>& inOwner)
		{
			ownClass = inClass;
			self = inSelf;
			owner = inOwner;
		}
		virtual void Construct() {}

		virtual bool Serialize(Archive& ar)
		{
			SerializeReflection(ar);
			return true;
		}

		Ptr<Object> GetOwner() const
		{
			return owner.Cast<Object>();
		}

		Ptr<Object> Self() const
		{
			return self.Cast<Object>();
		}

		template <typename T>
		Ptr<T> Self() const
		{
			return self.Cast<T>();
		}

		Refl::Class* GetType() const
		{
			return ownClass;
		}


		void SetName(Name newName)
		{
			name = MoveTemp(newName);
		}
		Name GetName() const
		{
			return name;
		}

		virtual Ptr<Context> GetContext() const;
	};


	// BEGIN - Pointer helpers
	template <typename T>
	using ObjectPtr = PtrOwner<T, ObjectBuilder>;

	template <class T>
	static CORE_API ObjectPtr<T> Create(Refl::Class* objectClass, const Ptr<Object> owner = {})
	{
		return MakeOwned<T, ObjectBuilder>(objectClass, owner);
	}

	template <class T>
	static CORE_API ObjectPtr<T> Create(const Ptr<Object> owner = {})
	{
		return MakeOwned<T, ObjectBuilder>(owner);
	}
	// END - Pointer helpers
}	 // namespace Rift
