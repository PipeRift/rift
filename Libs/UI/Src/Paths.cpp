// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/Paths.h"

namespace rift::Paths
{
	Path GetResourcesPath()
	{
		static p::Path relativeResourcesPath{"./Resources"};
		return GetBasePath() / relativeResourcesPath;
	}
};    // namespace rift::Paths
