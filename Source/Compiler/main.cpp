// Copyright 2015-2019 Piperift - All rights reserved

#include <cstdio>
#include <iostream>
#include "cxxopts.hpp"

#include "Pipeline.h"

int main(int argc, char* argv[])
{
	cxxopts::Options options("VCLang Compiler", "CLI Compiler for VCLang");
	options.add_options()
		("b,backend", "Back-end used to compile (C|LLVM)", cxxopts::value<std::string>()->default_value("C"))
		("o,output", "Destination of the compiled binaries") // a bool parameter
		("h,help", "Print usage");

	options.add_options("Optimization")
		("O1", "Optimization level 1")
		("O2", "Optimization level 2")
		("O3", "Optimization level 3");

	options.add_options("Debug")
		("v,verbose", "Verbosity level of the log. 0=Errors, 1=Warnings, 2=All", cxxopts::value<int>()->default_value("1"));

	auto result = options.parse(argc, argv);
	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		return 0;
	}

	return 0;
}
