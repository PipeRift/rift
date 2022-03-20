// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>
#include <Plugin.h>
#include <RiftContext.h>


namespace Rift
{
	class CPPBackendPlugin : public Plugin
	{
		CLASS(CPPBackendPlugin, Plugin)

	public:
		void Register(TPtr<RiftContext> context) override {}
	};


	namespace Compiler
	{
		class CppBackend : public Backend
		{
			CLASS(CppBackend, Backend)

		public:
			Name GetName() override
			{
				return "Cpp";
			}

			void Build(Context& context) override;
		};
	}    // namespace Compiler
}    // namespace Rift
