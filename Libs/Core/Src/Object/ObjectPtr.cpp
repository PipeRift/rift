// Copyright 2015-2020 Piperift - All rights reserved

#include "Object/ObjectPtr.h"

#include "Object/Object.h"


void BaseGlobalPtr::MoveFrom(BaseGlobalPtr&& other)
{
	if (!other.IsValid())
	{
		Reset();
		return;
	}

	ptr = MoveTemp(other.ptr);
	weaks = MoveTemp(other.weaks);

	// Update owned pointers to owner
	for (BaseWeakPtr* const weak : weaks)
	{
		// Not using set because objects have already been "rebinded"
		weak->globalPtr = this;
	}
}

BaseGlobalPtr::~BaseGlobalPtr()
{
	for (BaseWeakPtr* const weak : weaks)
	{
		weak->CleanOwner();
	}
}


// BaseWeakPtr::~BaseWeakPtr()

void BaseWeakPtr::Set(const BaseGlobalPtr* inGlobal)
{
	if (globalPtr == inGlobal)
		return;

	// Unbind from old global (if any)
	UnBind();

	globalPtr = inGlobal;

	// Bind into new owner
	if (globalPtr)
	{
		globalPtr->weaks.Add(this);
		id = (u32) globalPtr->weaks.Size() - 1;
	}
}

void BaseWeakPtr::MoveFrom(BaseWeakPtr&& other)
{
	if (globalPtr == other.globalPtr)
		return;

	// Unbind from old global (if any)
	UnBind();

	globalPtr = other.globalPtr;
	other.globalPtr = nullptr;
	if (globalPtr)
	{
		id = other.id;
		globalPtr->weaks[id] = this;
	}
}
