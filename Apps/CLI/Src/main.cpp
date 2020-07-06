
#include <Context.h>
#include <Project.h>

#include <Math/Vector.h>

using namespace VCLang;

int main()
{
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();

	Log::Message("aass");
	Log::Warning("warnnnn");
	Log::Error("errr");

	project->Init(Path("Project"));
	project->LoadAllAssets();
	return 0;
}
