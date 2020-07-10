// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Files/FileSystem.h"
#include "Log.h"
#include "Strings/String.h"


template <typename FileIterator = FileSystem::Iterator>
class FormatFileIterator
{
public:
	StringView format;
	FileIterator fileIterator;

	FormatFileIterator() noexcept = default;
	explicit FormatFileIterator(StringView format, const Path& path);

	FormatFileIterator(const FormatFileIterator&) noexcept = default;
	FormatFileIterator(FormatFileIterator&&) noexcept = default;
	~FormatFileIterator() noexcept = default;

	FormatFileIterator& operator=(const FormatFileIterator&) noexcept = default;
	FormatFileIterator& operator=(FormatFileIterator&&) noexcept = default;

	const fs::directory_entry& operator*() const noexcept
	{
		return *fileIterator;
	}

	const fs::directory_entry* operator->() const noexcept
	{
		return fileIterator.operator->();
	}

	FormatFileIterator& operator++()
	{
		FindNext();
		return *this;
	}

	FormatFileIterator& increment()
	{
		FindNext();
		return *this;
	}

	// other members as required by [input.iterators]:
	bool operator==(const FormatFileIterator& rhs) const noexcept
	{
		return fileIterator == rhs.fileIterator;
	}

	bool operator!=(const FormatFileIterator& rhs) const noexcept
	{
		return fileIterator != rhs.fileIterator;
	}

	bool IsEnd() const noexcept
	{
		return fileIterator._At_end();
	}

private:
	void FindNext();
};

template <typename FileIterator>
inline FormatFileIterator<FileIterator>::FormatFileIterator(StringView format, const Path& path)
	: format{format}
{
	if (!FileSystem::Exists(path) || !FileSystem::IsFolder(path))
	{
		return;
	}
	fileIterator = FileIterator(path);

	// Iterate to first found asset
	if (fileIterator->path().extension() != AssetManager::assetFormat)
	{
		FindNext();
	}
}

template <typename FileIterator>
inline void FormatFileIterator<FileIterator>::FindNext()
{
	static const FileIterator endIt{};
	std::error_code error;
	// Loop until end or until we find an asset
	while (true)
	{
		fileIterator.increment(error);
		if (fileIterator == endIt || fileIterator->path().extension() == format)
		{
			return;
		}
	}
}
