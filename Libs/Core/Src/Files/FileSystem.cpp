// Copyright 2015-2020 Piperift - All rights reserved

#include "Files/FileSystem.h"

#include "Profiler.h"
#include "Serialization/Archive.h"


bool FileSystem::FolderExists(const Path& path)
{
	return fs::exists(path) && fs::is_directory(path);
}

bool FileSystem::LoadJsonFile(Path path, Json& result)
{
	ScopedZone("LoadJsonFile", BB45D1);

	if (!Exists(path) || !IsFile(path))
	{
		return false;
	}

	std::ifstream file(path);
	result = {};
	try
	{
		file >> result;
	}
	catch (nlohmann::detail::exception ex)
	{
		Log::Error("Failed to parse json asset:\n{}", ex.what());
		return false;
	}
	return true;
}

bool FileSystem::SaveJsonFile(Path path, const Json& data, i32 indent)
{
	ScopedZone("SaveJsonFile", BB45D1);

	if (!IsFile(path))
	{
		return false;
	}

	std::ofstream file(path);
	if (indent >= 0)
		file << std::setw(indent) << data;
	else
		file << data;

	return true;
}


bool FileSystem::LoadStringFile(Path path, String& result)
{
	ScopedZone("LoadStringFile", BB45D1);

	if (!Exists(path) || !IsFile(path))
	{
		return false;
	}

	std::ifstream file(path);

	// Clean string and reserve it
	result = {};
	file.seekg(0, std::ios::end);
	result.reserve(i32(file.tellg()));
	file.seekg(0, std::ios::beg);

	// Improve by avoiding this copy from std::string to String
	std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
	result.assign(str.data(), str.size());

	return !result.empty();
}

bool FileSystem::SaveStringFile(Path path, const String& data)
{
	ScopedZone("SaveStringFile", BB45D1);

	if (!IsFile(path))
	{
		return false;
	}

	std::ofstream file(path);
	file.write(data.data(), data.size());
	return true;
}
