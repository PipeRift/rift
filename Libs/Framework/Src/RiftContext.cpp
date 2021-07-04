// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/FileWatcher.h"
#include "RiftContext.h"


namespace Rift
{
	TPtr<Module> RiftContext::OpenProject(Path path)
	{
		if (path.empty() || (rootProject && rootProject->GetPath() == path))
		{
			return {};
		}

		rootProject = Create<Module>();
		rootProject->Init(path);
		return rootProject.AsPtr();
	}

	void RiftContext::CloseProject()
	{
		// ast.Reset();
		rootProject.Delete();
	}

	TPtr<Module> RiftContext::GetAssetProject(TAssetPtr<TypeAsset> asset) const
	{
		return {};
	}
};    // namespace Rift
