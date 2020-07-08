
#include <Context.h>
#include <Math/Vector.h>
#include <Profiler.h>
#include <Project.h>


using namespace VCLang;

int main()
{
	ScopedZone("CLI Execution", 459bd1);
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();
	project->Init(Path("Project"));

	project->LoadAllAssets();

	std::chrono::duration<float, std::chrono::seconds::period> sleepPeriod{1.f};
	std::this_thread::sleep_for(sleepPeriod);
	return 0;
}
