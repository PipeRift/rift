// Copyright 2015-2023 Piperift - All rights reserved

#include "UI/Paths.h"


namespace rift::Paths
{
	p::String GetResourcesPath()
	{
		static p::StringView relativeResourcesPath{"./Resources"};
		return p::JoinPaths(p::GetBasePath(), relativeResourcesPath);
	}
};    // namespace rift::Paths
