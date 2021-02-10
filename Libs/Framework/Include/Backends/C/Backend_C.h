// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Backends/Backend.h"


namespace Rift::Backends::C
{
	class Backend_C : public Backend
	{
		CLASS(Backend_C, Backend)

		String code;

	public:
		virtual void OnCompile() override;

	protected:
		void Generate();
		void Build();
		virtual void OnCleanup() override;


		void AddInclude(StringView name);

		void ForwardDeclareClass(StringView name);
		void BeginClass(StringView name);
		void EndClass();

		void ForwardDeclareStruct(StringView name);
		void BeginStruct(StringView name);
		void EndStruct();
	};
}	 // namespace Rift::Backends::C
