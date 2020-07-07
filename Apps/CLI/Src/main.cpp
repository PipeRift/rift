
#include <Context.h>
#include <Math/Vector.h>
#include <Project.h>


using namespace VCLang;

int main()
{
	Context::Initialize();

	GlobalPtr<Project> project = Create<Project>();
	project->Init(Path("Project"));
	project->LoadAllAssets();
	return 0;
}
