// Copyright 2015-2023 Piperift - All rights reserved

#include "MIRBackendModule.h"

#include <AST/Utils/ModuleUtils.h>
#include <mir.h>
#include <NativeBindingModule.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Files.h>


namespace rift
{
	MIRBackendModule::MIRBackendModule()
	{
		AddDependency<NativeBindingModule>();
	}

	void MIRBackend::Build(Compiler& compiler)
	{
		ZoneScopedN("Backend: MIR");

		MIR_context_t ctx = MIR_init();

		if (!compiler.HasErrors())
		{
			p::Info("Build complete.");
		}
		else
		{
			p::Info("Build failed: {} errors", compiler.GetErrors().Size());
		}

		MIR_finish(ctx);
	}
}    // namespace rift