// Copyright 2015-2023 Piperift - All rights reserved

#include "LLVMBackendModule.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/IRGeneration.h"
#include "LLVMBackend/Linker.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Utils/ModuleUtils.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <NativeBindingModule.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Memory/NewDelete.h>

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


// TODO: Implement define module which checks if building statically. If static, don´t override
// new/delete
// P_OVERRIDE_NEW_DELETE


namespace rift
{
	LLVMBackendModule::LLVMBackendModule()
	{
		AddDependency<NativeBindingModule>();
	}

	namespace LLVM
	{
		void SaveModuleObject(Compiler& compiler, AST::Id moduleId,
		    llvm::TargetMachine* targetMachine, StringView targetTriple)
		{
			ZoneScoped;

			p::String intermediatesPath = p::ToString(compiler.config.intermediatesPath);
			// files::Delete(intermediatesPath, true, false);
			files::CreateFolder(intermediatesPath, true);

			auto& irModule      = compiler.ast.Get<CIRModule>(moduleId);
			irModule.objectFile = Strings::Format(
			    "{}/{}.o", intermediatesPath, AST::GetModuleName(compiler.ast, moduleId));
			p::Info("Creating object '{}'", irModule.objectFile);

			irModule.instance->setTargetTriple(ToLLVM(targetTriple));
			irModule.instance->setDataLayout(targetMachine->createDataLayout());

			std::error_code ec;
			llvm::raw_fd_ostream file(ToLLVM(irModule.objectFile), ec, llvm::sys::fs::OF_None);
			if (ec)
			{
				compiler.AddError(
				    Strings::Format("Could not open new object file: {}", ec.message()));
				irModule.objectFile = {};    // File not saved
				return;
			}

			llvm::legacy::PassManager pm;
			if (targetMachine->addPassesToEmitFile(pm, file, nullptr, llvm::CGFT_ObjectFile))
			{
				compiler.AddError("Target machine can't emit a file of this type");
				irModule.objectFile = {};    // File not saved
				return;
			}

			pm.run(*irModule.instance.Get());
			file.flush();
		}

		void CompileIR(Compiler& compiler, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
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
				compiler.AddError(error);
				return;
			}

			llvm::TargetOptions options;
			auto* targetMachine = target->createTargetMachine(
			    targetTriple, "generic", "", options, llvm::Optional<llvm::Reloc::Model>());

			// Emit LLVM IR to console
			for (AST::Id moduleId : ecs::ListAll<CIRModule>(compiler.ast))
			{
				const auto& irModule = compiler.ast.Get<const CIRModule>(moduleId).instance;
				irModule->print(llvm::outs(), nullptr);
			}

			for (AST::Id moduleId : ecs::ListAll<CIRModule>(compiler.ast))
			{
				LLVM::SaveModuleObject(compiler, moduleId, targetMachine, targetTriple);
			}
		}
	}    // namespace LLVM

	void LLVMBackend::Build(Compiler& compiler)
	{
		ZoneScopedN("Backend: LLVM");

		llvm::LLVMContext llvm;
		llvm::IRBuilder<> builder(llvm);

		p::Info("Generating LLVM IR");
		LLVM::GenerateIR(compiler, llvm, builder);
		if (compiler.HasErrors())
			return;    // TODO: Report errors here

		p::Info("Build IR");
		LLVM::CompileIR(compiler, llvm, builder);
		if (compiler.HasErrors())
			return;    // TODO: Report errors here

		LLVM::Link(compiler);

		compiler.ast.ClearPool<CIRModule>();

		if (!compiler.HasErrors())
		{
			p::Info("Build complete.");
		}
		else
		{
			p::Info("Build failed: {} errors", compiler.GetErrors().Size());
		}
	}
}    // namespace rift