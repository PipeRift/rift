// Copyright 2015-2022 Piperift - All rights reserved

#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <Compiler/Utils/BackendUtils.h>
#include <Context.h>
#include <Files/Paths.h>
#include <Profiler.h>
#include <RiftContext.h>

// Backends
#include <CppBackend.h>
#include <LLVMBackend.h>

#include <chrono>
#include <CLI/CLI.hpp>


using namespace Rift;


namespace Rift
{
	void AddBackendOption(
	    CLI::App& app, const TArray<TOwnPtr<Compiler::Backend>>& backends, String& selected)
	{
		String desc = "Backend to build with. Available: ";
		if (!backends.IsEmpty())
		{
			for (const auto& backend : backends)
			{
				Strings::FormatTo(desc, "{}, ", backend->GetName());
			}
			Strings::RemoveFromEnd(desc, 2);
		}
		else
		{
			Strings::FormatTo(desc, Name::None().ToString());
		}

		// Set default value
		const Name def = backends.IsEmpty() ? Name::None() : backends[0]->GetName();
		selected       = def.ToString();

		std::string stdDesc = Strings::Convert<std::string>(desc);
		app.add_option("-b,--backend", selected, stdDesc, true);
	}
}    // namespace Rift

int main(int argc, char** argv)
{
	CLI::App app{"Rift compiler"};
	String pathStr;
	app.add_option("-p,--project", pathStr, "Project path")->required();

	auto backends = Compiler::CreateBackends();

	String backendStr;
	Rift::AddBackendOption(app, backends, backendStr);

	CLI11_PARSE(app, argc, argv);

	ZoneScopedNC("CLI Execution", 0x459bd1);
	auto context = InitializeContext<RiftContext>();

	const Path path = Paths::FromString(pathStr);
	AST::Tree ast;
	Modules::OpenProject(ast, path);

	if (!Modules::HasProject(ast))
	{
		Log::Error("Couldn't open project '{}'", Paths::ToString(path));
		return 1;
	}

	Rift::Compiler::Config config;
	Rift::Compiler::Build(ast, config, Rift::Compiler::EBackend::Cpp);

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	ShutdownContext();
	return 0;
}
