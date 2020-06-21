// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"
#include "BaseType.h"


class StructType : public BaseType
{
public:

	/** Equivalent to "new Type()" */
	virtual BaseStruct* New() const = 0;


	// NOTE: Most of the class comparison functions do actually
	// call BaseType to reduce complexity and code duplication.
	//
	// We can cast safely to BaseType since Structs only inherit Structs

	StructType* GetParent() const { return static_cast<StructType*>(parent); }

	void GetAllChildren(TArray<StructType*>& outChildren) {
		__GetAllChildren(reinterpret_cast<TArray<BaseType*>&>(outChildren));
	}

	StructType* FindChild(Name StructTypeName) const {
		return static_cast<StructType*>(__FindChild(StructTypeName));
	}

	template<typename Type>
	bool IsChildOf() const { return BaseType::IsChildOf(Type::StaticStruct()); }

	bool IsA(StructType* other) const { return this == other; }
};
