// Copyright 2015-2023 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
//  Override as first include

#include <AST/Utils/ModuleUtils.h>
#include <ASTModule.h>
#include <Compiler/Compiler.h>
#include <Compiler/Utils/BackendUtils.h>
#include <GraphViewModule.h>
#include <MIRBackendModule.h>
#include <Pipe.h>
#include <Pipe/Files/Paths.h>
#include <Rift.h>

#include <chrono>
#include <thread>
#include <CLI/CLI.hpp>


using namespace rift;


namespace rift
{
	void AddBackendOption(CLI::App& app, const TArray<TOwnPtr<Backend>>& backends, String& selected)
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
			Strings::FormatTo(desc, "None");
		}

		// Set default value
		const Tag def = backends.IsEmpty() ? Tag::None() : backends[0]->GetName();
		selected      = def.AsString();

		auto stdDesc = Strings::Convert<std::string, TChar>(desc);
		app.add_option("-b,--backend", selected, stdDesc, true);
	}

	TPtr<Backend> FindBackendByName(const TArray<TOwnPtr<Backend>>& backends, Tag name)
	{
		TOwnPtr<Backend>* backend = backends.Find([name](const auto& backend) {
			return backend->GetName() == name;
		});
		if (backend)
		{
			return *backend;
		}
		return {};
	}
}    // namespace rift


int main(int argc, char** argv)
{
	p::Initialize("Saved/Logs");
	EnableModule<ASTModule>();
	EnableModule<MIRBackendModule>();
	EnableModule<GraphViewModule>();

	CompilerConfig config;

	CLI::App app{"Rift compiler"};
	String path;
	app.add_option("-p,--project", path, "Project path")->required();
	app.add_option("-v,--verbose", config.verbose, "Verbose")->required();
	app.add_flag("-O0{0},-O1{1},-O2{2},-O3{3}", config.optimization, "Optimization")
	    ->expected(0, 3);


	String selectedBackendStr;
	auto availableBackends = CreateBackends();
	AddBackendOption(app, availableBackends, selectedBackendStr);

	CLI11_PARSE(app, argc, argv);

	TPtr<Backend> backend = FindBackendByName(availableBackends, Tag(selectedBackendStr));

	AST::Tree ast;
	AST::OpenProject(ast, path);

	if (!AST::HasProject(ast))
	{
		p::Error("Couldn't open project '{}'", p::ToString(path));
		return 1;
	}

	Build(ast, config, backend);

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	p::Shutdown();
	return 0;
}
