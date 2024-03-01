// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/Settings.h"

#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


namespace rift
{
	p::String GetUserSettingsPath(p::StringView name)
	{
		static p::StringView relativeSettingsPath{"Rift"};
		p::String path =
		    p::JoinPaths(p::PlatformPaths::GetUserSettingsPath(), relativeSettingsPath, name);
		path.append(".json");
		return p::Move(path);
	}
}    // namespace rift
