// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Assets/ClassAsset.h"
#include "Backends/Backend.h"
#include "Project.h"

#include <CoreEngine.h>


namespace VCLang::Backends::C
{
	class Backend_C : public Backend
	{
		CLASS(Backend_C, Backend)

		Ptr<Project> project;

		void Generate(Ptr<Project> inProject);
		void Build();
	};
}	 // namespace VCLang::Backends::C
