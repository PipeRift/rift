// Copyright 2015-2022 Piperift - All rights reserved

#include "CppBackend.h"

#include "CppBackend/CMakeGen.h"
#include "CppBackend/Components/CCppNativeName.h"
#include "CppBackend/CppGeneration.h"

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Context.h>
#include <ECS/Filtering.h>
#include <Files/Files.h>
#include <Math/DateTime.h>


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

		void BuildCode(Context& context, const Pipe::Path& codePath, const Pipe::Path& cmakePath)
		{
			ZoneScoped;
			Files::CreateFolder(cmakePath, true);


			Log::Info("Generating");
			const String generate = Strings::Format(
			    "cmake -S {} -B {}", Pipe::ToString(codePath), Pipe::ToString(cmakePath));

			if (std::system(generate.c_str()) > 0)
			{
				context.AddError("Failed to generate cmake code");
				return;
			}

			Log::Info("Building");
			const String build = Strings::Format("cmake --build {} --config {}",
			    Pipe::ToString(cmakePath), context.config.buildMode);
			if (std::system(build.c_str()) > 0)
			{
				context.AddError("C++ build failed");
				return;
			}
		}
	}    // namespace Cpp

	void CppBackend::Build(Context& context)
	{
		ZoneScopedN("Backend: Cpp");

		DateTime startTime = DateTime::Now();

		const Pipe::Path& codePath = context.config.intermediatesPath / "Code";
		Files::Delete(codePath, true, false);
		Files::CreateFolder(codePath, true);

		Cpp::AssignNativeTypeNames(context.ast);

		Log::Info("Generating C++");
		Cpp::GenerateCode(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate C++ code");
			return;
		}


		Log::Info("Generating CMake");
		Cpp::GenerateCMake(context, codePath);
		if (context.HasErrors())
		{
			context.AddError("Failed to generate cmake files");
			return;
		}


		Log::Info("Building C++");
		const Pipe::Path cmakePath = context.config.intermediatesPath / "CMake";
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

		TAccess<CModule> modules{context.ast};
		for (AST::Id moduleId : ECS::ListAll<CModule>(modules))
		{
			Name name = Modules::GetModuleName(context.ast, moduleId);
			if (!Files::Copy(cmakePath / name.ToString() / context.config.buildMode,
			        context.config.binariesPath / name.ToString() / "Bin"))
			{
				context.AddError("Failed to copy binaries");
			}
		}

		const float duration = (DateTime::Now() - startTime).GetTotalSeconds();
		Log::Info("Build complete ({:.2f}s)", duration);
	}
}    // namespace Rift::Compiler
