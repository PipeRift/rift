// Copyright 2015-2022 Piperift - All rights reserved

#include "NativeBinding.h"

#include <AST/Id.h>
#include <clang-c/Index.h>
#include <Pipe/Core/Span.h>


namespace rift::NativeBinding
{
	struct ParsedModule
	{
		AST::Id moduleId;
		CXIndex index = clang_createIndex(0, 0);
		TArray<StringView> includes;
		TArray<CXTranslationUnit> units;

		~ParsedModule()
		{
			for (auto unit : units)
			{
				clang_disposeTranslationUnit(unit);
			}
			clang_disposeIndex(index);
		}
	};

	void ResolveIncludes(TSpan<ParsedModule> parsedModules) {}

	void ParseIncludes(TSpan<ParsedModule> parsedModules)
	{
		for (auto& module : parsedModules)
		{
			for (i32 i = 0; i < module.includes.Size(); ++i)
			{
				const StringView include     = module.includes[i];
				const CXTranslationUnit unit = clang_parseTranslationUnit(
				    module.index, include.data(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);
				if (!unit)
				{
					module.includes.RemoveAt(i, false);
					--i;
					Log::Error("Unable to parse module header '{}'", include);
					continue;
				}
				module.units.Add(unit);
			}
		}
	}

	void Sync()
	{
		TArray<AST::Id> moduleIds;
		// TODO: Find dirty native modules

		TArray<ParsedModule> parsedModules;
		parsedModules.Reserve(moduleIds.Size());
		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			auto& parsed    = parsedModules.AddDefaultedRef();
			parsed.moduleId = moduleIds[i];
		}
		ResolveIncludes(parsedModules);
		ParseIncludes(parsedModules);
		// TODO: Generate Rift interface
	}
}    // namespace rift::NativeBinding
