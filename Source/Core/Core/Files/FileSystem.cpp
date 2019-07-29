// Copyright 2015-2019 Piperift - All rights reserved

#include "FileSystem.h"
#include "Core/Serialization/Archive.h"
#include "Tools/Profiler.h"


bool FileSystem::FileExists(const Path& path)
{
	return fs::exists(path) && fs::is_regular_file(path);
}

bool FileSystem::FolderExists(const Path& path)
{
	return fs::exists(path) && fs::is_directory(path);
}

bool FileSystem::LoadJsonFile(Path path, Json& result)
{
	ScopedZone("LoadJsonFile", BB45D1);

	if (!SanitizeAssetPath(path) || !FileExists(path))
		return false;

	std::ifstream file(path);

	result = {};
	result << file;
	return true;
}

bool FileSystem::SaveJsonFile(Path path, const Json& data, i32 indent)
{
	ScopedZone("SaveJsonFile", BB45D1);

	if (!SanitizeAssetPath(path))
		return false;

	std::ofstream file(path);
	if(indent >= 0)
		file << std::setw(indent) << data;
	else
		file << data;

	return true;
}


bool FileSystem::LoadStringFile(Path path, String& result)
{
	ScopedZone("LoadStringFile", BB45D1);

	if (!SanitizeAssetPath(path) || !FileExists(path))
		return false;

	std::ifstream file(path);

	// Clean string and reserve it
	result = {};
	file.seekg(0, std::ios::end);
	result.reserve(i32(file.tellg()));
	file.seekg(0, std::ios::beg);

	// Improve by avoiding this copy from std::string to String
	std::string str{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
	result.assign(str.data(), str.size());

	return !result.empty();
}

bool FileSystem::SaveStringFile(Path path, const String& data)
{
	ScopedZone("SaveStringFile", BB45D1);

	if (!SanitizeAssetPath(path))
		return false;

	std::ofstream file(path);
	file.write(data.data(), data.size());
	return true;
}

Path FileSystem::GetAssetsPath()
{
	// Take two folders up. May change for distributed versions / other platforms
	Path path = fs::current_path().parent_path().parent_path();
	path /= "Assets";
	return eastl::move(path);
}

void FileSystem::RelativeToAssetsPath(Path& path)
{
	if (path.is_absolute())
	{
		path = fs::relative(path, GetAssetsPath());
	}
}

Path FileSystem::GetConfigPath()
{
	// Take two folders up. May change for distributed versions / other platforms
	Path path = fs::current_path().parent_path().parent_path();
	path /= "Config";
	return eastl::move(path);
}

Path FileSystem::FindMetaFile(Path in)
{
	if (!SanitizeAssetPath(in))
		return {};

	if (in.extension() != ".meta")
		in += ".meta";

	return in;
}

Path FileSystem::FindRawFile(Path in)
{
	if (!SanitizeAssetPath(in))
		return {};

	const Path stem = in.stem();
	if (!stem.has_extension() || in.extension() != ".meta")
		return {};

	return in.parent_path() / stem;
}

bool FileSystem::IsAssetPath(Path path)
{
	if (path.is_relative())
		return true;

	const Path assets = GetAssetsPath();
	const Path rootPath = path.root_path();

	// Iterate parent directories to find Assets path
	for (; path != rootPath; path = path.parent_path())
	{
		if (path == assets)
		{
			return true;
		}
	}
	return false;
}
