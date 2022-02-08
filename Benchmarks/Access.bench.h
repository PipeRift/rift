// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "nanobench.h"

#include <AST/Filtering.h>
#include <AST/Tree.h>
#include <Misc/Time.h>


using namespace ankerl;
using namespace Rift;

struct Test1
{};
struct Test2
{};
struct Test3
{};


void RunAccessBenchmarks()
{
	{
		ankerl::nanobench::Bench bench;
		bench.title("Iteration - Two included")
		    .performanceCounters(true)
		    .minEpochIterations(20)
		    .maxEpochTime(Rift::Seconds{3});

		AST::Tree ast2;
		TArray<AST::Id> ids2;
		ids2.Resize(100000);
		ast2.Create(ids2);
		ast2.Add<Test1, Test2>(ids2);
		ast2.Add<Test3>(TSpan<AST::Id>(ids2, 50000));
		bench.relative(true).run("Filter/View", [&ast2] {
			auto filter = ast2.Filter<Test1, Test2>();
			for (AST::Id id : filter)
			{
				ankerl::nanobench::doNotOptimizeAway(id);
			}
		});

		AST::Tree ast;
		TArray<AST::Id> ids;
		ids.Resize(100000);
		ast.Create(ids);
		ast.Add<Test1, Test2>(ids);
		ast.Add<Test3>(TSpan<AST::Id>(ids, 50000));

		bench.run("Access", [&ast] {
			AST::TAccess<Test1, Test2> access{ast};
			for (AST::Id id : AST::ListAll<Test1, Test2>(access))
			{
				ankerl::nanobench::doNotOptimizeAway(id);
			}
		});
	}

	{
		ankerl::nanobench::Bench bench;
		bench.title("Iteration - Two included, one excluded")
		    .performanceCounters(true)
		    .minEpochIterations(20)
		    .maxEpochTime(Rift::Seconds{3});

		AST::Tree ast2;
		TArray<AST::Id> ids2;
		ids2.Resize(100000);
		ast2.Create(ids2);
		ast2.Add<Test1, Test2>(ids2);
		ast2.Add<Test3>(TSpan<AST::Id>(ids2, 50000));
		bench.relative(true).run("Filter/View", [&ast2] {
			auto filter = ast2.Filter<Test1, Test2>(AST::TExclude<Test3>{});
			for (AST::Id id : filter)
			{
				ankerl::nanobench::doNotOptimizeAway(id);
			}
		});

		AST::Tree ast;
		TArray<AST::Id> ids;
		ids.Resize(100000);
		ast.Create(ids);
		ast.Add<Test1, Test2>(ids);
		ast.Add<Test3>(TSpan<AST::Id>(ids, 50000));
		bench.run("Access", [&ast] {
			AST::TAccess<Test1, Test2, Test3> access{ast};
			auto ids = AST::ListAll<Test1, Test2>(access);
			AST::RemoveIf<Test3>(access, ids);
			for (AST::Id id : ids)
			{
				ankerl::nanobench::doNotOptimizeAway(id);
			}
		});
	}
}
