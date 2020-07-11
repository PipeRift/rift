// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Assets/ClassAsset.h"
#include "Backends/Backend.h"

#include <CoreEngine.h>


namespace VCLang::Backends::C
{
	class Backend_C : public Backend
	{
		CLASS(Backend_C, Backend)

	public:
		virtual void OnCompile() override;

	protected:
		void Generate();
		void Build();
		virtual void OnCleanup() override;
	};
}	 // namespace VCLang::Backends::C
