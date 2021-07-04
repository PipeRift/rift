// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Module.h"

#include <Context.h>


namespace Rift
{
	/**
	 * A solution represents an open project and all its build dependencies.
	 */
	class RiftContext : public Context
	{
		CLASS(RiftContext, Context)

	private:
		TOwnPtr<Module> rootProject;
		// TArray<TOwnPtr<Project>> subProjects;


	public:
		TPtr<Module> OpenProject(Path path);
		void CloseProject();

		bool HasProject() const
		{
			return rootProject.IsValid();
		}

		TPtr<Module> GetRootProject() const
		{
			return rootProject;
		}

		TPtr<Module> GetAssetProject(TAssetPtr<TypeAsset> asset) const;

		// Handy helper to obtain the root project
		static TPtr<Module> GetProject()
		{
			return GetContext<RiftContext>()->GetRootProject();
		}
	};
}    // namespace Rift
