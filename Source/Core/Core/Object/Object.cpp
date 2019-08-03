// Copyright 2015-2019 Piperift - All rights reserved

#include "Object.h"
#include "Context.h"

Ptr<Context> Object::GetContext() const
{
	return Context::Get();
}
