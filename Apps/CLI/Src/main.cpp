
#include <Context.h>
#include <Profiler.h>
#include <Project.h>

#include <chrono>

using namespace VCLang;


int main(int argc, char** argv)
{
	ScopedZone("CLI Execution", 459bd1);
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();
	project->Init(Path("Project"));

	project->LoadAllAssets();

	while (true)
	{
		// Live for a second to let the profiler connect. Temporal
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	return 0;
}
