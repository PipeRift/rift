
#include <Context.h>
#include <Project.h>

using namespace VCLang;

int main()
{
	Context::Initialize();
	auto project = Create<Project>();
	project->Init(".");
	project->LoadAllAssets();
	return 0;
}
