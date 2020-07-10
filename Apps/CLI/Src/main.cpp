
#include <Context.h>
#include <Profiler.h>
#include <Project.h>


using namespace VCLang;

int main()
{
	String v = "hey";
	StringView s = v;

	ScopedZone("CLI Execution", 459bd1);
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();
	project->Init(Path("Project"));

	project->LoadAllAssets();

	// Live for a second to let the profiler connect. Temporal
	std::chrono::duration<float, std::chrono::seconds::period> sleepPeriod{1.f};
	std::this_thread::sleep_for(sleepPeriod);
	return 0;
}
