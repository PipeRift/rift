// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <PCore/Array.h>
#include <PCore/Name.h>
#include <PCore/String.h>



namespace rift::Compiler::LLVM
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
}    // namespace rift::Compiler::LLVM
