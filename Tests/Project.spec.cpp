// Copyright 2015-2023 Piperift - All rights reserved

#include <AST/Components/CModule.h>
#include <AST/Systems/LoadSystem.h>
#include <AST/Utils/ModuleUtils.h>
#include <bandit/bandit.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Memory/OwnPtr.h>

#include <chrono>
#include <thread>


using namespace snowhouse;
using namespace bandit;
using namespace rift;
using namespace p;
using namespace std::chrono_literals;

String testProjectPath = p::JoinPaths(GetCurrentPath(), "TestProject");


go_bandit([]() {
	describe("Project", []() {
		before_each([]() {
			files::Delete(testProjectPath, true, false);

			if (!files::ExistsAsFolder(testProjectPath))
			{
				files::CreateFolder(testProjectPath);
			}
		});

		after_each([]() {
			files::Delete(testProjectPath);
		});

		it("Can load empty descriptor", [&]() {
			files::SaveStringFile(p::JoinPaths(testProjectPath, AST::moduleFilename), "{}");

			AST::Tree ast;
			bool result = AST::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(AST::HasProject(ast), Equals(true));
		});

		it("Project name equals the folder", [&]() {
			files::SaveStringFile(p::JoinPaths(testProjectPath, AST::moduleFilename), "{}");

			AST::Tree ast;
			bool result = AST::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(AST::HasProject(ast), Equals(true));

			StringView projectName = AST::GetProjectName(ast).AsString();
			AssertThat(projectName, Equals("TestProject"));
		});

		// TODO: Fix module loading. They can't load from CFileRef pointing to the folder and not
		// the file
		it("Project name can be overriden", [&]() {
			files::SaveStringFile(
			    p::JoinPaths(testProjectPath, AST::moduleFilename), "{\"name\": \"SomeProject\"}");

			AST::Tree ast;
			bool result = AST::OpenProject(ast, testProjectPath);
			AssertThat(result, Equals(true));
			AssertThat(AST::HasProject(ast), Equals(true));

			StringView projectName = AST::GetProjectName(ast).AsString();
			AssertThat(projectName, Equals("SomeProject"));
		});
	});
});
