// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/Settings.h"

#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


namespace rift
{
	p::String GetUserSettingsPath()
	{
		static p::StringView relativeSettingsPath{"Rift"};
		return p::JoinPaths(p::PlatformPaths::GetUserSettingsPath(), relativeSettingsPath);
	}
}    // namespace rift
