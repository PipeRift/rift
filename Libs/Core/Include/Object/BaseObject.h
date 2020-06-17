// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Core/Memory/Allocator.h"
#include "Core/Reflection/Reflection.h"
#include "CoreEngine.h"
#include "CoreTypes.h"

#include <EASTL/type_traits.h>



class Class;
class Object;

template <typename Type>
class Ptr;


class CORE_API BaseObject : public BaseStruct
{
protected:
	BaseObject() : BaseStruct()
	{
	}

public:
	void StartDestroy()
	{
		BeforeDestroy();
	}

	Class* GetClass() const;
	Ptr<Object> Self() const;

protected:
	virtual void BeforeDestroy()
	{
	}
};
