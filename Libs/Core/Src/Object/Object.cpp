// Copyright 2015-2020 Piperift - All rights reserved

#include "Object/Object.h"

#include "Context.h"


namespace Rift
{
	Ptr<Context> Object::GetContext() const
	{
		return Context::Get();
	}
}	 // namespace Rift
