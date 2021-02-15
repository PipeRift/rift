// Copyright 2015-2021 Piperift - All rights reserved

#include <Context.h>
#include <Files/FileSystem.h>
#include <Memory/OwnPtr.h>
#include <Project.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;

Path testProjectPath = FileSystem::GetCurrent() / "TestProject";


go_bandit([]() {
	describe("Project", []() {
		before_each([]() {
			Context::Initialize();
			FileSystem::Delete(testProjectPath, true, false);

			if (!FileSystem::ExistsAsFolder(testProjectPath))
			{
				FileSystem::CreateFolder(testProjectPath);
			}
		});

		after_each([]() {
			FileSystem::Delete(testProjectPath);
		});

		it("Can load empty descriptor", [&]() {
			Json json{{"asset_type", "ProjectAsset"}};
			FileSystem::SaveJsonFile(testProjectPath / "Project.rf", json);

			OwnPtr<Project> project = Create<Project>();
			AssertThat(project.IsValid(), Equals(true));

			project->Init(testProjectPath);
			AssertThat(project->IsValid(), Equals(true));
		});
	});
});
