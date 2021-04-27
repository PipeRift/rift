// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include <CoreObject.h>


namespace Rift
{
	/**
	 * A solution represents an open project and all its build dependencies.
	 */
	class Solution : public Object
	{
		CLASS(Solution, Object)

		TOwnPtr<Project> mainProject;

		ECS::Registry astRegistry;


		void BuildAll() {}
	};
}    // namespace Rift
