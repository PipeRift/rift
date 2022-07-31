// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>
#include <Plugin.h>
#include <Rift.h>


namespace rift
{
	class CPPBackendPlugin : public Plugin
	{
		CLASS(CPPBackendPlugin, Plugin)

	public:
		void Register(TPtr<Rift> rift) override {}
	};


	namespace compiler
	{
		class CppBackend : public Backend
		{
			CLASS(CppBackend, Backend)

		public:
			Name GetName() override
			{
				return "Cpp";
			}

			void Build(Compiler& compiler) override;
		};
	}    // namespace compiler
}    // namespace rift
