// Copyright 2015-2022 Piperift - All rights reserved

#include "UI/Paths.h"

namespace Rift::Paths
{
	Path GetResourcesPath()
	{
		static Path relativeResourcesPath{"./Resources"};
		return GetBasePath() / relativeResourcesPath;
	}
};    // namespace Rift::Paths
