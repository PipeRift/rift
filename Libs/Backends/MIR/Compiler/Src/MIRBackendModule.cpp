// Copyright 2015-2023 Piperift - All rights reserved

#include "MIRBackendModule.h"

#include "C2MIR.h"
#include "Compiler/CompilerConfig.h"
#include "IRGeneration.h"

#include <AST/Utils/ModuleUtils.h>
#include <NativeBindingModule.h>
#include <Pipe/Core/Log.h>
#include <Pipe/Files/Files.h>
#include <PipeTime.h>
#include <stdlib.h>


extern "C"
{
#include <mir-gen.h>
#include <mir.h>
}


#ifndef _WIN32
	#include <dlfcn.h>
	#if defined(__unix__) || defined(__APPLE__)
		#include <sys/stat.h>
	#endif
#else
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif


namespace rift
{
	struct Lib
	{
		const char* name = nullptr;
		void* handler    = nullptr;
	};

	// clang-format off
	static Lib gStdLibs[] {
	#if defined(_WIN32)
		{"C:\\Windows\\System32\\msvcrt.dll"},
		{"C:\\Windows\\System32\\kernel32.dll"},
		{"C:\\Windows\\System32\\ucrtbase.dll"},
	#elif defined(__APPLE__)
		{"/usr/lib/libc.dylib"},
		{"/usr/lib/libm.dylib"},
	#elif defined(__unix__)
		#if UINTPTR_MAX == 0xffffffff
			{"/lib/libc.so.6"},
			{"/lib32/libc.so.6"},
			{"/lib/libm.so.6"},
			{"/lib32/libm.so.6"},
			{"/lib/libpthread.so.0"},
			{"/lib32/libpthread.so.0"},
		#elif UINTPTR_MAX == 0xffffffffffffffff
			#if defined(__x86_64__)
				{"/lib64/libc.so.6"},
				{"/lib/x86_64-linux-gnu/libc.so.6"},
				{"/lib64/libm.so.6"},
				{"/lib/x86_64-linux-gnu/libm.so.6"},
				{"/usr/lib64/libpthread.so.0"},
				{"/lib/x86_64-linux-gnu/libpthread.so.0"},
				{"/usr/lib/libc.so"},
			#elif (__aarch64__)
				{"/lib64/libc.so.6"}, {"/lib/aarch64-linux-gnu/libc.so.6"},
				{"/lib64/libm.so.6"},
				{"/lib/aarch64-linux-gnu/libm.so.6"},
				{"/lib64/libpthread.so.0"},
				{"/lib/aarch64-linux-gnu/libpthread.so.0"},
			#elif (__PPC64__)
				{"/lib64/libc.so.6"},
				{"/lib64/libm.so.6"},
				{"/lib64/libpthread.so.0"},
				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					{"/lib/powerpc64le-linux-gnu/libc.so.6"},
					{"/lib/powerpc64le-linux-gnu/libm.so.6"},
					{"/lib/powerpc64le-linux-gnu/libpthread.so.0"},
				#else
					{"/lib/powerpc64-linux-gnu/libc.so.6"},
					{"/lib/powerpc64-linux-gnu/libm.so.6"},
					{"/lib/powerpc64-linux-gnu/libpthread.so.0"},
				#endif
			#elif (__s390x__)
				{"/lib64/libc.so.6"},
				{"/lib/s390x-linux-gnu/libc.so.6"},
				{"/lib64/libm.so.6"},
				{"/lib/s390x-linux-gnu/libm.so.6"},
				{"/lib64/libpthread.so.0"},
				{"/lib/s390x-linux-gnu/libpthread.so.0"},
			#elif (__riscv)
				{"/lib64/libc.so.6"},
				{"/lib/riscv64-linux-gnu/libc.so.6"},
				{"/lib64/libm.so.6"},
				{"/lib/riscv64-linux-gnu/libm.so.6"},
				{"/lib64/libpthread.so.0"},
				{"/lib/riscv64-linux-gnu/libpthread.so.0"},
			#else
				#error Cannot recognize 32- or 64-bit target
			#endif
		#endif
	#endif
	};

	#if defined(__unix__)
	static const char* gStdLibDirs[]
	{
		#if UINTPTR_MAX == 0xffffffff
			"/lib",
			"/lib32"
		#elif UINTPTR_MAX == 0xffffffffffffffff
			#if defined(__x86_64__)
				"/lib64",
				"/lib/x86_64-linux-gnu"
			#elif (__aarch64__)
				"/lib64",
				"/lib/aarch64-linux-gnu"
			#elif (__PPC64__)
				"/lib64",
				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					"/lib/powerpc64le-linux-gnu",
				#else
					"/lib/powerpc64-linux-gnu",
				#endif
			#elif (__s390x__)
				"/lib64",
				"/lib/s390x-linux-gnu"
			#elif (__riscv)
				"/lib64",
				"/lib/riscv64-linux-gnu"
			#endif
		#endif
	};
	static const char* gLibSuffix = ".so";
	#elif defined(__APPLE__)
	static const char* gStdLibDirs[]{"/usr/lib"};
	static const char* gLibSuffix    = ".dylib";
	#elif defined(_WIN32)
	static const char* gStdLibDirs[]{"C:\\Windows\\System32"};
	static const char* gLibSuffix = ".dll";
	#define dlopen(n, f) LoadLibrary(n)
	#define dlclose(h) FreeLibrary(HMODULE(h))
	#define dlsym(h, s) GetProcAddress(HMODULE(h), s)
	#endif
	// clang-format on


	static void OpenSTDLibs()
	{
		for (Lib& stdLib : gStdLibs)
		{
			stdLib.handler = dlopen(stdLib.name, RTLD_LAZY);
		}
	}

	static void CloseSTDLibs()
	{
		for (Lib& stdLib : gStdLibs)
		{
			if (stdLib.handler)
			{
				dlclose(stdLib.handler);
			}
		}
	}

	static void* ImportResolver(const char* name)
	{
		void *handler, *sym = nullptr;

		for (Lib& stdLib : gStdLibs)
		{
			handler = stdLib.handler;
			if (handler)
			{
				sym = dlsym(handler, name);
				if (sym)
				{
					break;
				}
			}
		}

		if (sym == nullptr)
		{
#ifdef _WIN32
			if (strcmp(name, "LoadLibrary") == 0)
				return LoadLibrary;
			if (strcmp(name, "FreeLibrary") == 0)
				return FreeLibrary;
			if (strcmp(name, "GetProcAddress") == 0)
				return GetProcAddress;
#else
			if (strcmp(name, "dlopen") == 0)
				return (void*)dlopen;
			if (strcmp(name, "dlerror") == 0)
				return (void*)dlerror;
			if (strcmp(name, "dlclose") == 0)
				return (void*)dlclose;
			if (strcmp(name, "dlsym") == 0)
				return (void*)dlsym;
			if (strcmp(name, "stat") == 0)
				return (void*)stat;
			if (strcmp(name, "lstat") == 0)
				return (void*)lstat;
			if (strcmp(name, "fstat") == 0)
				return (void*)fstat;
	#if defined(__APPLE__) && defined(__aarch64__)
			if (strcmp(name, "__nan") == 0)
				return __nan;
			if (strcmp(name, "_MIR_set_code") == 0)
				return _MIR_set_code;
	#endif
#endif
			fprintf(stderr, "can not load symbol %s\n", name);
			CloseSTDLibs();
			exit(1);
		}
		return sym;
	}


	MIRBackendModule::MIRBackendModule()
	{
		AddDependency<NativeBindingModule>();
	}

	using EntryFunctionPtr = p::i32 (*)();
	void MIRBackend::Build(Compiler& compiler)
	{
		MIR::GenerateC(compiler);

		MIR_context* ctx = MIR_init();

		MIR::CToMIR(compiler, ctx);

		p::i32 nGen = 1;
		MIR_item_t func, mainFunc = nullptr;

		{    // Find main
			for (MIR_module_t module       = DLIST_HEAD(MIR_module_t, *MIR_get_module_list(ctx));
			     module != nullptr; module = DLIST_NEXT(MIR_module_t, module))
			{
				for (func = DLIST_HEAD(MIR_item_t, module->items); func != nullptr;
				     func = DLIST_NEXT(MIR_item_t, func))
				{
					if (func->item_type == MIR_func_item && strcmp(func->u.func->name, "main") == 0)
					{
						mainFunc = func;
					}
				}
				MIR_load_module(ctx, module);
			}
		}

		OpenSTDLibs();
		MIR_gen_init(ctx, nGen);
		for (p::i32 i = 0; i < nGen; ++i)
		{
			if (compiler.config.optimization != OptimizationLevel::Zero)
			{
				MIR_gen_set_optimize_level(ctx, i, (unsigned)compiler.config.optimization);
			}
			// if (gen_debug_level >= 0)
			//{
			//	MIR_gen_set_debug_file(ctx, i, stderr);
			//	MIR_gen_set_debug_level(ctx, i, gen_debug_level);
			// }
		}
		MIR_link(
		    ctx, nGen > 1 ? MIR_set_parallel_gen_interface : MIR_set_gen_interface, ImportResolver);

		auto entry = EntryFunctionPtr(nGen > 1 ? MIR_gen(ctx, 0, mainFunc) : mainFunc->addr);

		p::DateTime startTime = p::DateTime::Now();
		p::i32 resultCode     = entry();    // Run!
		if (compiler.config.verbose)
		{
			p::Info("  execution       -- {:.3f}s\n",
			    (p::DateTime::Now() - startTime).GetTotalSeconds());
			p::Info("exit code: {}\n", resultCode);
		}
		MIR_gen_finish(ctx);

		if (!compiler.HasErrors())
		{
			p::Info("Build complete.");
		}
		else
		{
			p::Info("Build failed: {} errors", compiler.GetErrors().Size());
		}

		CloseSTDLibs();
		MIR_finish(ctx);
	}
}    // namespace rift