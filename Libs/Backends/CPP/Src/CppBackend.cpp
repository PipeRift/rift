// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend.h"

#include "CppBackend/CMakeGen.h"
#include "CppBackend/CodeGen.h"
#include "CppBackend/Components/CCppNativeName.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Context.h>
#include <Files/Files.h>


namespace Rift::Compiler
{
	namespace Cpp
	{
		void AssignNativeTypeNames(AST::Tree& ast)
		{
			const auto& nativeTypes = ast.GetNativeTypes();
			ast.Add<CCppNativeName>(nativeTypes.boolId, {"bool"});
			ast.Add<CCppNativeName>(nativeTypes.floatId, {"float"});
			ast.Add<CCppNativeName>(nativeTypes.doubleId, {"double"});
			ast.Add<CCppNativeName>(nativeTypes.u8Id, {"std::uint_8"});
			ast.Add<CCppNativeName>(nativeTypes.i8Id, {"std::int_8"});
			ast.Add<CCppNativeName>(nativeTypes.u16Id, {"std::uint_16"});
			ast.Add<CCppNativeName>(nativeTypes.i16Id, {"std::int_16"});
			ast.Add<CCppNativeName>(nativeTypes.u32Id, {"std::uint_32"});
			ast.Add<CCppNativeName>(nativeTypes.i32Id, {"std::int_32"});
			ast.Add<CCppNativeName>(nativeTypes.u64Id, {"std::uint_64"});
			ast.Add<CCppNativeName>(nativeTypes.i64Id, {"std::int_64"});
			ast.Add<CCppNativeName>(nativeTypes.stringId, {"std::string"});
		}

		void BuildCode(Context& context, const Path& codePath, const Path& cmakePath)
		{
			ZoneScopedC(0x459bd1);
			Files::CreateFolder(cmakePath, true);


			Log::Info("Generating");
			const String generate = Strings::Format(
			    "cmake -S {} -B {}", Paths::ToString(codePath), Paths::ToString(cmakePath));

			if (std::system(generate.c_str()) > 0)
			{
				context.AddError("Failed to generate cmake code");
				return;
			}

			Log::Info("Building");
			const String build = Strings::Format("cmake --build {} --config {}",
			    Paths::ToString(cmakePath), context.config.buildMode);
			if (std::system(build.c_str()) > 0)
			{
				context.AddError("C++ build failed");
				return;
			}
		}

		void BuildModule(Context& context, AST::Id moduleId)
		{
			if (!context.ast.Has<CModule>(moduleId))
			{
				Log::Info("Cant find module");
				return;
			}

			Log::Info("Building module '{}'", Modules::GetModuleName(context.ast, moduleId));
			Log::Info("Loading module files");
			// project->LoadAllAssets();
		}
	}    // namespace Cpp

	void CppBackend::Build(Context& context)
	{
		ZoneScopedC(0x459bd1);

		Log::Info("Building project '{}'", Modules::GetProjectName(context.ast));


		Cpp::AssignNativeTypeNames(context.ast);


		const Path& codePath = context.config.intermediatesPath / "Code";
		Files::Delete(codePath, true, false);
		Files::Delete(context.config.binariesPath, true, false);
		Files::CreateFolder(codePath, true);
		Files::CreateFolder(context.config.binariesPath, true);

		Log::Info("Generating Code: C++");
		Cpp::GenerateCode(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate C++ code");
			return;
		}


		Log::Info("Generating code: CMake");
		Cpp::GenerateCMake(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate cmake files");
			return;
		}


		Log::Info("Building C++");
		const Path cmakePath = context.config.intermediatesPath / "CMake";
		Cpp::BuildCode(context, codePath, cmakePath);
		if (context.HasErrors())
		{
			context.AddError("Compilation failed while building C++ code");
			return;
		}


		// Copy code & binaries
		if (!Files::Copy(codePath, context.config.binariesPath))
		{
			context.AddError("Failed to copy code");
			return;
		}
		auto modules = context.ast.Filter<CModule>();
		for (AST::Id moduleId : modules)
		{
			Name name = Modules::GetModuleName(context.ast, moduleId);
			if (!Files::Copy(cmakePath / name.ToString() / context.config.buildMode,
			        context.config.binariesPath / name.ToString() / "Bin"))
			{
				context.AddError("Failed to copy binaries");
			}
		}

		Log::Info("Build complete");
	}
}    // namespace Rift::Compiler
