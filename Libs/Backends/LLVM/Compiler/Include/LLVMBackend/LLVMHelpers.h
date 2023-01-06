// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <Pipe/Core/Array.h>
#include <Pipe/Core/Name.h>
#include <Pipe/Core/String.h>


namespace rift::compiler::LLVM
{
	using namespace p::core;

	inline llvm::StringRef ToLLVM(StringView string)
	{
		return {string.data(), string.size()};
	}
	inline llvm::StringRef ToLLVM(const String& string)
	{
		return ToLLVM(StringView{string});
	}
	inline llvm::StringRef ToLLVM(Name name)
	{
		return ToLLVM(StringView{name.ToString()});
	}

	template<typename T>
	inline llvm::ArrayRef<T> ToLLVM(const TArray<T>& array)
	{
		return {array.Data(), sizet(array.Size())};
	}
}    // namespace rift::compiler::LLVM
