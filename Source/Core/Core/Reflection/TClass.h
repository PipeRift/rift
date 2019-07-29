// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "Class.h"
#include "TProperty.h"


// Class will be specialized for each type at compile time and store
// the metadata for that type.
template <typename Type>
class TClass : public Class {
	static_assert(eastl::is_convertible< Type, BaseObject >::value, "Type is not an Object!");
private:

	static TClass _class;


public:

	TClass() : Class() {
		Type::__meta_Registry();
		Type::__meta_RegistryProperties();
	}

public:

	virtual GlobalPtr<BaseObject> CreateInstance(const Ptr<BaseObject>& owner) override {
		GlobalPtr<BaseObject> ptr = GlobalPtr<Type>::Create(owner);
		return ptr;
	}

	static TClass* GetStatic() { return &_class; }
};

template <typename Type>
TClass<Type> TClass<Type>::_class {};
