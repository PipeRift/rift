// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Project.h"

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
		AST::Tree ast;
		TOwnPtr<Project> rootProject;
		// TArray<TOwnPtr<Project>> subProjects;


	public:
		TPtr<Project> OpenProject(Path path);
		void CloseProject();

		bool HasProject() const
		{
			return rootProject.IsValid();
		}

		TPtr<Project> GetRootProject() const
		{
			return rootProject;
		}

		TPtr<Project> GetAssetProject(TAssetPtr<TypeAsset> asset) const;

		// Handy helper to obtain the root project
		static TPtr<Project> GetProject()
		{
			return GetContext<RiftContext>()->GetRootProject();
		}

		static AST::Tree* GetAST()
		{
			const TPtr<Project> project = GetProject();
			return project ? &project->GetAST() : nullptr;
		}
	};
}    // namespace Rift
