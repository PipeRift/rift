// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/AssetManager.h"
#include "Files/FormatFileIterator.h"


template <bool bRecursive = false>
class AssetIterator : public FormatFileIterator<FileSystem::Iterator>
{
public:
	using FormatFileIterator<FileSystem::Iterator>::FormatFileIterator;

	explicit AssetIterator(const Path& path) : FormatFileIterator(AssetManager::assetFormat, path) {}
};

template <>
class AssetIterator<true> : public FormatFileIterator<FileSystem::RecursiveIterator>
{
public:
	using FormatFileIterator<FileSystem::RecursiveIterator>::FormatFileIterator;

	explicit AssetIterator(const Path& path) : FormatFileIterator(AssetManager::assetFormat, path) {}
};


template <bool bRecursive>
inline AssetIterator<bRecursive> begin(AssetIterator<bRecursive> it) noexcept
{
	return it;
}

template <bool bRecursive>
inline AssetIterator<bRecursive> end(AssetIterator<bRecursive>) noexcept
{
	return {};
}
