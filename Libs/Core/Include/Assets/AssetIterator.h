// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/AssetInfo.h"
#include "Assets/AssetManager.h"
#include "Files/FormatFileIterator.h"


template <bool bRecursive = false>
class AssetIterator : public FormatFileIterator<FileSystem::Iterator>
{
public:
	mutable AssetInfo currentAsset;


	using FormatFileIterator<FileSystem::Iterator>::FormatFileIterator;

	explicit AssetIterator(const Path& path) : FormatFileIterator(AssetManager::assetFormat, path) {}

	const AssetInfo& operator*() const noexcept
	{
		currentAsset = {fileIterator->path()};
		return currentAsset;
	}

	const AssetInfo* operator->() const noexcept
	{
		return &operator*();
	}
};

template <>
class AssetIterator<true> : public FormatFileIterator<FileSystem::RecursiveIterator>
{
public:
	mutable AssetInfo currentAsset;


	using FormatFileIterator<FileSystem::RecursiveIterator>::FormatFileIterator;

	explicit AssetIterator(const Path& path) : FormatFileIterator(AssetManager::assetFormat, path) {}

	const AssetInfo& operator*() const noexcept
	{
		currentAsset = {fileIterator->path()};
		return currentAsset;
	}

	const AssetInfo* operator->() const noexcept
	{
		return &operator*();
	}
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
