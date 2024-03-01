// Copyright 2015-2023 Piperift - All rights reserved

#include "UI/Paths.h"

#include <Pipe/Files/PlatformPaths.h>


namespace rift::Paths
{
	p::String GetResourcesPath()
	{
		static p::StringView relativeResourcesPath{"Resources"};
		return p::JoinPaths(p::PlatformPaths::GetBasePath(), relativeResourcesPath);
	}
};    // namespace rift::Paths
