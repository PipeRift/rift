// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "BaseObject.h"
#include "ObjectPtr.h"
#include "../Reflection/TClass.h"
#include "../Serialization/Archive.h"


class World;


class Object : public BaseObject {
	ORPHAN_CLASS(Object)

private:

	PROP(Name, name)
	Name name;

	Ptr<BaseObject> self;
	Class* ownClass;
	Ptr<BaseObject> owner;


public:

	Object() : BaseObject(), self{}, ownClass{ nullptr }, owner{} {};

	void PreConstruct(Ptr<BaseObject>&& inSelf, Class* inClass, const Ptr<BaseObject>& inOwner) {
		ownClass = inClass;
		self = inSelf;
		owner = inOwner;
	}
	virtual void Construct() {}

	virtual bool Serialize(Archive& ar) {
		SerializeReflection(ar);
		return true;
	}

	/** Manual destruction is dangerous and a bad practice
	void Destroy()
	{
		if (BaseGlobalPtr* global = Self().__GetGlobal())
		{
			global->Reset();
		}
	}*/


	Ptr<Object> GetOwner() const { return owner.Cast<Object>(); }

	Ptr<Object> Self() const { return self.Cast<Object>(); }

	template<typename T>
	Ptr<T> Self() const { return self.Cast<T>(); }

	Class* GetClass() const { return ownClass; }


	void SetName(Name newName) { name = eastl::move(newName); }
	Name GetName() const { return name; }

	virtual Ptr<World> GetWorld() const;
};



template <typename Type>
using IsObject = eastl::is_convertible<Type, Object>;

template<class ObjectType>
static GlobalPtr<ObjectType> Create(Class* objectClass, const Ptr<Object> owner = {}) {
	static_assert(IsObject<ObjectType>::value, "Type is not an Object!");

	if (objectClass)
	{
		return objectClass->CreateInstance(owner).Cast<ObjectType>();
	}
	return {};
}

template<class ObjectType>
static inline GlobalPtr<ObjectType> Create(const Ptr<Object> owner = {}) {
	static_assert(IsObject<ObjectType>::value, "Type is not an Object!");

	return GlobalPtr<ObjectType>::Create(owner);
}
