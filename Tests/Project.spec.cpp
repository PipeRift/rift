// Copyright 2015-2022 Piperift - All rights reserved

#include <AST/Systems/LoadSystem.h>
#include <AST/Utils/ModuleUtils.h>
#include <bandit/bandit.h>
#include <Context.h>
#include <Files/Files.h>
#include <Files/Paths.h>
#include <Memory/OwnPtr.h>

#include <chrono>
#include <thread>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Pipe;
using namespace std::chrono_literals;

Path testProjectPath = GetCurrentPath() / "TestProject";


go_bandit([]() {
	describe("Project", []() {
		before_each([]() {
			Files::Delete(testProjectPath, true, false);

			if (!Files::ExistsAsFolder(testProjectPath))
			{
				Files::CreateFolder(testProjectPath);
			}
		});

		after_each([]() {
			Files::Delete(testProjectPath);
		});

		it("Can load empty descriptor", [&]() {
			Files::SaveStringFile(testProjectPath / Modules::moduleFile, "{}");

			AST::Tree ast;
			bool result = Modules::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(Modules::HasProject(ast), Equals(true));
		});

		it("Project name equals the folder", [&]() {
			Files::SaveStringFile(testProjectPath / Modules::moduleFile, "{}");

			AST::Tree ast;
			bool result = Modules::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(Modules::HasProject(ast), Equals(true));

			StringView projectName = Modules::GetProjectName(ast).ToString();
			AssertThat(projectName, Equals("TestProject"));
		});

		// TODO: Fix module loading. They can't load from CFileRef pointing to the folder and not
		// the file
		it("Project name can be overriden", [&]() {
			Files::SaveStringFile(
			    testProjectPath / Modules::moduleFile, "{\"name\": \"SomeProject\"}");

			AST::Tree ast;
			bool result = Modules::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(Modules::HasProject(ast), Equals(true));

			StringView projectName = Modules::GetProjectName(ast).ToString();
			AssertThat(projectName, Equals("SomeProject"));
		});
	});
});
