// Copyright 2015-2023 Piperift - All rights reserved

#include "NativeBindingModule.h"

#include "Components/CDeclCStatic.h"
#include "Components/CDeclCStruct.h"
#include "Components/CNativeBinding.h"
#include "HeaderIterator.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Id.h>
#include <AST/Tree.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/TypeUtils.h>
#include <clang-c/Index.h>
#include <Pipe/PipeArrays.h>
#include <Pipe/PipeECS.h>


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
		// Register types
		AST::RiftTypeSettings typeSettings{.category = "Bindings"};
		typeSettings.displayName  = "C Struct";
		typeSettings.hasVariables = true;
		typeSettings.hasFunctions = false;
		AST::RegisterFileType<CDeclCStruct>("CStruct", typeSettings);
		typeSettings.displayName       = "C Static";
		typeSettings.hasVariables      = true;
		typeSettings.hasFunctions      = true;
		typeSettings.hasFunctionBodies = false;
		AST::RegisterFileType<CDeclCStatic>("CStatic", typeSettings);
		AST::PreAllocPools<CDeclCStruct, CDeclCStatic>();

		// Register module binding
		AST::RegisterModuleBinding(
		    {.id = "C", .tagType = CNativeBinding::GetStaticType(), .displayName = "C"});
		AST::RegisterSerializedModulePools<CNativeBinding>();
		AST::PreAllocPools<CNativeBinding>();
	}

	void FindHeaders(AST::Tree& ast, TView<ParsedModule> parsedModules)
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

	void ParseHeaders(AST::Tree& ast, TView<ParsedModule> parsedModules)
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
					p::Error("Unable to parse module header '{}'", include);
					continue;
				}
				module.units.Add(unit);
			}
		}
	}

	void NativeBindingModule::SyncIncludes(AST::Tree& ast)
	{
		TArray<AST::Id> moduleIds;
		p::FindAllIdsWith<AST::CModule, CNativeBinding>(ast, moduleIds);

		// Only use automatic native bindings on modules marked as such
		moduleIds.RemoveIfSwap([ast](auto id) {
			return !ast.Get<CNativeBinding>(id).autoGenerateDefinitions;
		});

		TArray<ParsedModule> parsedModules;
		parsedModules.Reserve(moduleIds.Size());
		for (i32 i = 0; i < moduleIds.Size(); ++i)
		{
			auto& parsed = parsedModules.AddRef();
			parsed.id    = moduleIds[i];
		}
		FindHeaders(ast, parsedModules);
		ParseHeaders(ast, parsedModules);
		// TODO: Generate Rift interface
	}
}    // namespace rift
