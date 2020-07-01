
#include <Framework.h>
#include <Project.h>

using namespace VCLang;

int main()
{
	Framework lang{};

	lang.GetAssetManager();
	auto project = Create<Project>();
	return 0;
}
