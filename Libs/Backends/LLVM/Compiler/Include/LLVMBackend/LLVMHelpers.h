// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <Pipe/Core/String.h>
#include <Pipe/Core/Tag.h>
#include <Pipe/PipeArrays.h>


namespace rift::LLVM
{
	using namespace p::core;

	inline llvm::StringRef ToLLVM(p::StringView string)
	{
		return {string.data(), string.size()};
	}
	inline llvm::StringRef ToLLVM(const p::String& string)
	{
		return ToLLVM(p::StringView{string});
	}
	inline llvm::StringRef ToLLVM(p::Tag name)
	{
		return ToLLVM(p::StringView{name.AsString()});
	}

	template<typename T>
	inline llvm::ArrayRef<T> ToLLVM(const p::IArray<T>& array)
	{
		return {array.Data(), sizet(array.Size())};
	}
}    // namespace rift::LLVM
