// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Types/Class.h>


namespace Rift::Compiler
{
	class Backend : public Class
	{
		CLASS(Backend, Class)

	public:
		virtual Name GetName()
		{
			return Name::None();
		}
	};
}    // namespace Rift::Compiler
