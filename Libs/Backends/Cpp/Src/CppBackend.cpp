// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend.h"

#include "CppBackend/CMakeGen.h"
#include "CppBackend/Components/CCppNativeName.h"
#include "CppBackend/CppGeneration.h"

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Math/DateTime.h>


namespace rift::compiler
{
	namespace Cpp
	{
		void AssignNativeTypeNames(AST::Tree& ast)
		{
			const auto& nativeTypes = ast.GetNativeTypes();
			ast.Add<CCppNativeName>(nativeTypes.boolId, "bool");
			ast.Add<CCppNativeName>(nativeTypes.floatId, "float");
			ast.Add<CCppNativeName>(nativeTypes.doubleId, "double");
			ast.Add<CCppNativeName>(nativeTypes.u8Id, "std::uint_8");
			ast.Add<CCppNativeName>(nativeTypes.i8Id, "std::int_8");
			ast.Add<CCppNativeName>(nativeTypes.u16Id, "std::uint_16");
			ast.Add<CCppNativeName>(nativeTypes.i16Id, "std::int_16");
			ast.Add<CCppNativeName>(nativeTypes.u32Id, "std::uint_32");
			ast.Add<CCppNativeName>(nativeTypes.i32Id, "std::int_32");
			ast.Add<CCppNativeName>(nativeTypes.u64Id, "std::uint_64");
			ast.Add<CCppNativeName>(nativeTypes.i64Id, "std::int_64");
			ast.Add<CCppNativeName>(nativeTypes.stringId, "std::string");
		}

		void BuildCode(Compiler& compiler, const p::Path& codePath, const p::Path& cmakePath)
		{
			ZoneScoped;
			files::CreateFolder(cmakePath, true);


			Log::Info("Generating");
			const String generate =
			    Strings::Format("cmake -S {} -B {}", p::ToString(codePath), p::ToString(cmakePath));

			if (std::system(generate.c_str()) > 0)
			{
				compiler.AddError("Failed to generate cmake code");
				return;
			}

			Log::Info("Building");
			const String build = Strings::Format(
			    "cmake --build {} --config {}", p::ToString(cmakePath), compiler.config.buildMode);
			if (std::system(build.c_str()) > 0)
			{
				compiler.AddError("C++ build failed");
				return;
			}
		}
	}    // namespace Cpp

	void CppBackend::Build(Compiler& compiler)
	{
		ZoneScopedN("Backend: Cpp");

		DateTime startTime = DateTime::Now();

		const p::Path& codePath = compiler.config.intermediatesPath / "Code";
		files::Delete(codePath, true, false);
		files::CreateFolder(codePath, true);

		Cpp::AssignNativeTypeNames(compiler.ast);

		Log::Info("Generating C++");
		Cpp::GenerateCode(compiler, codePath);
		if (compiler.HasErrors())
		{
			compiler.AddError("Failed to generate C++ code");
			return;
		}


		Log::Info("Generating CMake");
		Cpp::GenerateCMake(compiler, codePath);
		if (compiler.HasErrors())
		{
			compiler.AddError("Failed to generate cmake files");
			return;
		}


		Log::Info("Building C++");
		const p::Path cmakePath = compiler.config.intermediatesPath / "CMake";
		Cpp::BuildCode(compiler, codePath, cmakePath);
		if (compiler.HasErrors())
		{
			compiler.AddError("Compilation failed while building C++ code");
			return;
		}


		// Copy code & binaries
		if (!files::Copy(codePath, compiler.config.binariesPath))
		{
			compiler.AddError("Failed to copy code");
			return;
		}

		TAccess<CModule> modules{compiler.ast};
		for (AST::Id moduleId : ecs::ListAll<CModule>(modules))
		{
			Name name = Modules::GetModuleName(compiler.ast, moduleId);
			if (!files::Copy(cmakePath / name.ToString() / compiler.config.buildMode,
			        compiler.config.binariesPath / name.ToString() / "Bin"))
			{
				compiler.AddError("Failed to copy binaries");
			}
		}

		const float duration = (DateTime::Now() - startTime).GetTotalSeconds();
		Log::Info("Build complete ({:.2f}s)", duration);
	}
}    // namespace rift::compiler
