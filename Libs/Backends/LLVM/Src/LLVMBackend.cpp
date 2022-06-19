// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/IRGeneration.h"
#include "LLVMBackend/Linker.h"
#include "LLVMBackend/LLVMHelpers.h"
#include "Pipe/Core/Log.h"

#include <AST/Utils/ModuleUtils.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <Pipe/Files/Files.h>

#if LLVM_VERSION_MAJOR >= 14
#	include <llvm/MC/TargetRegistry.h>
#else
#	include <llvm/Support/TargetRegistry.h>
#endif
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>


namespace rift::Compiler
{
	namespace LLVM
	{
		void SaveModuleObject(Context& context, AST::Id moduleId,
		    llvm::TargetMachine* targetMachine, StringView targetTriple)
		{
			ZoneScoped;

			p::String intermediatesPath = p::ToString(context.config.intermediatesPath);
			// files::Delete(intermediatesPath, true, false);
			files::CreateFolder(intermediatesPath, true);


			auto& irModule      = context.ast.Get<CIRModule>(moduleId);
			irModule.objectFile = Strings::Format(
			    "{}/{}.o", intermediatesPath, Modules::GetModuleName(context.ast, moduleId));
			Log::Info("Creating object '{}'", irModule.objectFile);

			irModule.instance->setTargetTriple(ToLLVM(targetTriple));
			irModule.instance->setDataLayout(targetMachine->createDataLayout());

			std::error_code ec;
			llvm::raw_fd_ostream file(ToLLVM(irModule.objectFile), ec, llvm::sys::fs::OF_None);
			if (ec)
			{
				context.AddError(
				    Strings::Format("Could not open new object file: {}", ec.message()));
				irModule.objectFile = {};    // File not saved
				return;
			}

			llvm::legacy::PassManager pm;
			if (targetMachine->addPassesToEmitFile(pm, file, nullptr, llvm::CGFT_ObjectFile))
			{
				context.AddError("Target machine can't emit a file of this type");
				irModule.objectFile = {};    // File not saved
				return;
			}

			pm.run(*irModule.instance.Get());
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
			for (AST::Id moduleId : ecs::ListAll<CIRModule>(context.ast))
			{
				const auto& irModule = context.ast.Get<const CIRModule>(moduleId).instance;
				irModule->print(llvm::outs(), nullptr);
			}

			for (AST::Id moduleId : ecs::ListAll<CIRModule>(context.ast))
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

		Log::Info("Linking");
		LLVM::Link(context);
	}
}    // namespace rift::Compiler