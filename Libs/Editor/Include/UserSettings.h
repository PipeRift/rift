
// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Memory/OwnPtr.h>


namespace rift
{

	class UserSettings : p::Class
	{
	private:
		static TOwnPtr<UserSettings> instance;

		static UserSettings* Get();
	};
}    // namespace rift
