
#include <bandit/bandit.h>
#include <Context.h>


class NoFileLogContext : public p::Context
{
public:
	NoFileLogContext() : p::Context(p::Path{}) {}
};


int main(int argc, char* argv[])
{
	p::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	p::ShutdownContext();
}
