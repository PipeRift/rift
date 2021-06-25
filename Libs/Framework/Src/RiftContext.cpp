// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/FileWatcher.h"
#include "RiftContext.h"


namespace Rift
{
	TPtr<Project> RiftContext::OpenProject(Path path)
	{
		if (path.empty() || (rootProject && rootProject->GetPath() == path))
		{
			return {};
		}

		rootProject = Create<Project>();
		rootProject->Init(path);
		return rootProject.AsPtr();
	}

	void RiftContext::CloseProject()
	{
		rootProject.Delete();
	}

	TPtr<Project> RiftContext::GetAssetProject(TAssetPtr<TypeAsset> asset) const
	{
		return {};
	}
};    // namespace Rift
