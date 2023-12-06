// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Compiler/Backend.h>
#include <Module.h>

struct MIR_context;
struct c2mir_options;


namespace rift
{
	struct Input;
	class MIRBackendModule : public Module
	{
		P_CLASS(MIRBackendModule, Module)

	public:
		MIRBackendModule();
	};


	class MIRBackend : public Backend
	{
		P_CLASS(MIRBackend, Backend)

	public:
		Tag GetName() override
		{
			return "MIR";
		}

		void Build(Compiler& compiler) override;
	};
}    // namespace rift
