// Copyright 2015-2024 Piperift - All rights reserved
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
	public:
		using Super = Module;
		P_CLASS(MIRBackendModule)

	public:
		MIRBackendModule();
	};


	class MIRBackend : public Backend
	{
	public:
		using Super = Backend;
		P_CLASS(MIRBackend)

	public:
		Tag GetName() override
		{
			return "MIR";
		}

		void Build(Compiler& compiler) override;

		void PrintBuildFinish(Compiler& compiler) const;
	};
}    // namespace rift
