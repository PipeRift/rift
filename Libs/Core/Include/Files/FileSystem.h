// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Containers/Array.h"
#include "CoreEngine.h"
#include "Serialization/Json.h"
#include "Strings/String.h"

#include <filesystem>


namespace VCLang
{
	namespace fs = std::filesystem;

	using Path = fs::path;

	class Archive;

	class FileSystem
	{
	public:
		using Iterator = fs::directory_iterator;
		using RecursiveIterator = fs::recursive_directory_iterator;
		using SpaceInfo = fs::space_info;


		/** String API */

		static bool FileExists(const String& path)
		{
			return FileExists(FromString(path));
		}
		static bool FolderExists(const String& path)
		{
			return FolderExists(FromString(path));
		}
		static bool LoadJsonFile(const String& path, Json& result)
		{
			return LoadJsonFile(FromString(path), result);
		}
		static bool SaveJsonFile(const String& path, const Json& data, i32 indent = -1)
		{
			return SaveJsonFile(FromString(path), data, indent);
		}
		static bool LoadStringFile(const String& path, String& result)
		{
			return LoadStringFile(FromString(path), result);
		}
		static bool SaveStringFile(const String& path, const String& data)
		{
			return SaveStringFile(FromString(path), data);
		}


		/** Path API */

		static bool FileExists(const Path& path);
		static bool FolderExists(const Path& path);


		static bool LoadJsonFile(Path path, Json& result);
		static bool SaveJsonFile(Path path, const Json& data, i32 indent = -1);

		static bool LoadStringFile(Path path, String& result);
		static bool SaveStringFile(Path path, const String& data);

		static void CreateFolder(const Path& path, bool bRecursive = false)
		{
			if (!path.empty() && !path.has_extension() && !Exists(path))
			{
				if (bRecursive)
				{
					CreateFolder(path.parent_path(), bRecursive);
				}
				fs::create_directory(path);
			}
		}

		static Iterator CreateIterator(const Path& path)
		{
			if (!IsFolder(path))
			{
				return {};
			}
			return Iterator(path);
		}
		static RecursiveIterator CreateRecursiveIterator(const Path& path)
		{
			if (!Exists(path) || !IsFolder(path))
			{
				return {};
			}
			return RecursiveIterator(path);
		}

		/** HELPERS */

		static bool Exists(const String& path)
		{
			return Exists(FromString(path));
		}

		static bool Exists(const Path& path)
		{
			return fs::exists(path);
		}

		static bool IsFolder(const String& path)
		{
			return IsFolder(FromString(path));
		}

		static bool IsFolder(const Path& path)
		{
			return fs::is_directory(path);
		}

		static bool IsFile(const String& path)
		{
			return IsFile(FromString(path));
		}

		static bool IsFile(const Path& path)
		{
			return fs::is_regular_file(path);
		}

		static Path ToRelative(const Path& path, const Path& parent = GetCurrent())
		{
			return fs::relative(path, parent);
		}

		static Path ToAbsolute(const Path& path, const Path& parent = GetCurrent())
		{
			if (path.is_absolute())
			{
				return {};
			}
			return parent / path;
		}

		static bool IsInside(const Path& base, const Path& parent)
		{
			return !ToRelative(base, parent).empty();
		}

		static Path GetCurrent()
		{
			return fs::current_path();
		}

		static String ToString(const Path& path)
		{
			return path.string();
		}

		static Path FromString(StringView pathStr)
		{
			Path path;
			path.assign(pathStr);
			return path;
		}

	private:
		SpaceInfo Space(Path target)
		{
			return fs::space(target);
		}
	};
}	 // namespace VCLang
