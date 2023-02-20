// Copyright 2015-2023 Piperift - All rights reserved

#include "NativeBindingModule.h"

#include "Components/CDeclCStatic.h"
#include "Components/CDeclCStruct.h"
#include "Components/CNativeBinding.h"
#include "HeaderIterator.h"
#include "Rift.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Id.h>
#include <AST/Tree.h>
#include <AST/Utils/ModuleUtils.h>
#include <clang-c/Index.h>
#include <Pipe/Core/Span.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Memory/NewDelete.h>


// P_OVERRIDE_NEW_DELETE


namespace rift
{
	struct ParsedModule
	{
		AST::Id id    = AST::NoId;
		CXIndex index = clang_createIndex(0, 0);
		TArray<String> headers;
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

	void NativeBindingModule::Load()
	{
		AST::RegisterModuleBinding(
		    {.id = "C", .tagType = CNativeBinding::GetStaticType(), .displayName = "C"});
		RegisterRiftType<CDeclCStruct>("CStruct", {.displayName     = "C Struct",
		                                              .category     = "Bindings",
		                                              .hasVariables = true,
		                                              .hasFunctions = false});
		RegisterRiftType<CDeclCStatic>("CStatic", {.displayName          = "C Static",
		                                              .category          = "Bindings",
		                                              .hasVariables      = true,
		                                              .hasFunctions      = true,
		                                              .hasFunctionBodies = false});
	}

	void FindHeaders(AST::Tree& ast, TSpan<ParsedModule> parsedModules)
	{
		p::TAccess<AST::CFileRef> access{ast};
		for (auto& module : parsedModules)
		{
			Path path = AST::GetModulePath(access, module.id);
			for (const auto& headerPath : HeaderIterator(path))
			{
				module.headers.Add(p::ToString(headerPath));
			}
		}
	}

	void ParseHeaders(AST::Tree& ast, TSpan<ParsedModule> parsedModules)
	{
		for (auto& module : parsedModules)
		{
			for (i32 i = 0; i < module.headers.Size(); ++i)
			{
				const StringView include = module.headers[i];
				const CXTranslationUnit unit =
				    clang_parseTranslationUnit(module.index, include.data(), nullptr, 0, nullptr, 0,
				        CXTranslationUnit_DetailedPreprocessingRecord);
				if (!unit)
				{
					module.headers.RemoveAt(i, false);
					--i;
					Log::Error("Unable to parse module header '{}'", include);
					continue;
				}
				module.units.Add(unit);
			}
		}
	}

	void NativeBindingModule::SyncIncludes(AST::Tree& ast)
	{
		TArray<AST::Id> moduleIds;
		p::ecs::ListAll<AST::CModule, CNativeBinding>(ast, moduleIds);

		TArray<ParsedModule> parsedModules;
		parsedModules.Reserve(moduleIds.Size());
		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			auto& parsed = parsedModules.AddDefaultedRef();
			parsed.id    = moduleIds[i];
		}
		FindHeaders(ast, parsedModules);
		ParseHeaders(ast, parsedModules);
		// TODO: Generate Rift interface
	}
}    // namespace rift
