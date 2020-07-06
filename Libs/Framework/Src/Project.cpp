// Copyright 2015-2020 Piperift - All rights reserved

#include "Project.h"


void VCLang::Project::LoadAllAssets()
{
    for (const auto& entry : FileSystem::CreateRecursiveIterator(projectPath))
    {
        Log::Message(FileSystem::ToString(entry.path()));
    }
}

Path VCLang::Project::ToProjectPath(const Path& path)
{
    if (path.is_relative())
    {
        return projectPath / path;
    }
    return FileSystem::IsInside(path, projectPath)? path : Path{};
}