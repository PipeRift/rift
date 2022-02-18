// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/IRGeneration.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Filtering.h>
#include <AST/Utils/ModuleUtils.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>


namespace Rift::Compiler
{
	namespace LLVM
	{
		void SaveModuleObject(Context& context, AST::Id moduleId,
		    llvm::TargetMachine* targetMachine, StringView targetTriple)
		{
			ZoneScoped;

			const String filePath =
			    Strings::Format("{}/{}.o", Paths::ToString(context.config.intermediatesPath),
			        Modules::GetModuleName(context.ast, moduleId));
			Log::Info("Creating object '{}'", filePath);

			const auto& irModule = context.ast.Get<const CIRModule>(moduleId).instance;

			irModule->setTargetTriple(targetTriple);
			irModule->setDataLayout(targetMachine->createDataLayout());

			std::error_code ec;
			llvm::raw_fd_ostream file(ToLLVM(filePath), ec, llvm::sys::fs::OF_None);
			if (ec)
			{
				context.AddError(
				    Strings::Format("Could not open new object file: {}", ec.message()));
				return;
			}

			llvm::legacy::PassManager pm;
			if (targetMachine->addPassesToEmitFile(pm, file, nullptr, llvm::CGFT_ObjectFile))
			{
				context.AddError("Target machine can't emit a file of this type");
				return;
			}

			pm.run(*irModule.Get());
			file.flush();
		}

		void CompileIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
		{
			ZoneScoped;
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmParser();
			llvm::InitializeNativeTargetAsmPrinter();
			std::string targetTriple = llvm::sys::getDefaultTargetTriple();

			std::string error;
			const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
			if (!target)
			{
				context.AddError(error);
				return;
			}

			llvm::TargetOptions options;
			auto* targetMachine = target->createTargetMachine(
			    targetTriple, "generic", "", options, llvm::Optional<llvm::Reloc::Model>());

			// Emit LLVM IR to console
			for (AST::Id moduleId : AST::ListAll<CIRModule>(context.ast))
			{
				const auto& irModule = context.ast.Get<const CIRModule>(moduleId).instance;
				irModule->print(llvm::outs(), nullptr);
			}

			const Path bitCodePath = context.config.intermediatesPath / "LLVM";
			for (AST::Id moduleId : AST::ListAll<CIRModule>(context.ast))
			{
				LLVM::SaveModuleObject(context, moduleId, targetMachine, targetTriple);
			}
		}
	}    // namespace LLVM

	void LLVMBackend::Build(Context& context)
	{
		ZoneScopedN("Backend: LLVM");

		llvm::LLVMContext llvm;
		llvm::IRBuilder<> builder(llvm);

		Log::Info("Generating LLVM IR");
		LLVM::GenerateIR(context, llvm, builder);

		Log::Info("Build IR");
		LLVM::CompileIR(context, llvm, builder);
	}
}    // namespace Rift::Compiler