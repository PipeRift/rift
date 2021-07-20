// Copyright 2015-2021 Piperift - All rights reserved

#include <AST/Utils/ModuleUtils.h>
#include <Context.h>
#include <Files/Files.h>
#include <Memory/OwnPtr.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;

Path testProjectPath = Paths::GetCurrent() / "TestProject";


go_bandit([]() {
	describe("Project", []() {
		before_each([]() {
			InitializeContext();
			Files::Delete(testProjectPath, true, false);

			if (!Files::ExistsAsFolder(testProjectPath))
			{
				Files::CreateFolder(testProjectPath);
			}
		});

		after_each([]() {
			Files::Delete(testProjectPath);
			ShutdownContext();
		});

		it("Can load empty descriptor", [&]() {
			Files::SaveStringFile(
			    testProjectPath / Modules::projectFile, "{\"asset_type\": \"ModuleAsset\"}");

			AST::Tree ast = Modules::OpenProject(testProjectPath);
			AssertThat(Modules::HasProject(ast), Equals(true));
		});

		it("Project name equals the folder", [&]() {
			Files::SaveStringFile(
			    testProjectPath / Modules::projectFile, "{\"asset_type\": \"ModuleAsset\"}");

			AST::Tree ast = Modules::OpenProject(testProjectPath);
			AssertThat(Modules::HasProject(ast), Equals(true));

			AssertThat(Modules::GetProjectName(ast), Equals(Name{"TestProject"}));
		});

		it("Project name can be overriden", [&]() {
			Files::SaveStringFile(testProjectPath / Modules::projectFile,
			    "{\"asset_type\": \"ModuleAsset\", \"name\": \"SomeProject\"}");

			AST::Tree ast = Modules::OpenProject(testProjectPath);
			AssertThat(Modules::HasProject(ast), Equals(true));

			AssertThat(Modules::GetProjectName(ast), Equals(Name{"SomeProject"}));
		});
	});
});
