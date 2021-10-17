
#include <Context.h>
#include <bandit/bandit.h>


class NoFileLogContext : public Rift::Context
{
public:
	NoFileLogContext() : Rift::Context(Rift::Path{}) {}
};


int main(int argc, char* argv[])
{
	Rift::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	Rift::ShutdownContext();
}
