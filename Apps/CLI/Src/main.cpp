// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeNewDelete.h>
//  Override as first include

#include <AST/Utils/ModuleUtils.h>
#include <ASTModule.h>
#include <Compiler/Compiler.h>
#include <Compiler/Utils/BackendUtils.h>
#include <GraphViewModule.h>
#include <MIRBackendModule.h>
#include <Pipe.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>
#include <PipeTime.h>
#include <Rift.h>

#include <chrono>
#include <CLI/CLI.hpp>
#include <thread>


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

		auto stdDesc = Strings::Convert<std::string, char>(desc);
		app.add_option("-b,--backend", selected, stdDesc)->capture_default_str();
	}

	TPtr<Backend> FindBackendByName(const TArray<TOwnPtr<Backend>>& backends, Tag name)
	{
		TOwnPtr<Backend>* backend = backends.FindIf([name](const auto& backend)
		{
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
	p::Logger logger = p::Logger{.infoCallback =
	                                 [](StringView msg)
	{
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Info] {}\n", msg);
		std::cout << text;
	},
	    .warningCallback =
	        [](StringView msg)
	{
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Warning] {}\n", msg);
		std::cout << text;
	},
	    .errorCallback = [](StringView msg)
	{
		String text;
		auto now = p::DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		p::Strings::FormatTo(text, "[Error] {}\n", msg);
		std::cout << text;
	}};

	p::Initialize(&logger);
	p::Info(p::PlatformPaths::GetUserSettingsPath());
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

	ast::Tree ast;
	ast::OpenProject(ast, path);

	if (!ast::HasProject(ast))
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
