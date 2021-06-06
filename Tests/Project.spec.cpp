// Copyright 2015-2021 Piperift - All rights reserved

#include <Context.h>
#include <Files/Files.h>
#include <Memory/OwnPtr.h>
#include <Project.h>
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
			    testProjectPath / "Project.rf", "{\"asset_type\": \"ProjectType\"}");

			TOwnPtr<Project> project = Create<Project>();
			AssertThat(project.IsValid(), Equals(true));

			project->Init(testProjectPath);
			AssertThat(project->IsValid(), Equals(true));
		});
	});
});
