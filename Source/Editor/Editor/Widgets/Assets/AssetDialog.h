#pragma once

#include "Core/Files/FileSystem.h"
#include "Core/Misc/DateTime.h"


enum class EDialogSortOrder : u8 {
	Unsorted,
	SortDown,
	SortUp
};

enum class EDialogSortType : u8 {
	Name,
	Size,
	Date
};

enum class EDialogResult : u8 {
	Success,
	Cancel,
	None
};

struct AssetDialogFile
{
	Path path;
	u64 size;
	DateTime dateTime;
	bool bSelected;
};

struct AssetDialogSorter
{
	EDialogSortOrder order = EDialogSortOrder::Unsorted;
	EDialogSortType  type  = EDialogSortType::Name;

	bool operator()(const AssetDialogFile& a, const AssetDialogFile& b) const
	{
		switch (type)
		{
		case EDialogSortType::Name:
			return SortType(a.path, b.path);
		case EDialogSortType::Size:
			return !SortType(a.size, b.size);
		case EDialogSortType::Date:
			return SortType(a.dateTime, b.dateTime);
		}
		return false;
	}

	void SwitchToNextOrder()
	{
		// Increment enum by 1 looping
		order = EDialogSortOrder((u8(order) + 1) % 3);
	}

	const char* GetOrderSymbol(EDialogSortType inType) const
	{
		if (type == inType)
		{
			switch (order)
			{
			case EDialogSortOrder::SortDown: return "v";//R"(▾)";
			case EDialogSortOrder::SortUp:   return "^";//R"(▴)";
			}
		}
		return " ";
	}

private:

	template<typename T>
	inline bool SortType(const T& a, const T& b) const
	{
		switch (order)
		{
		case EDialogSortOrder::SortDown: return a < b;
		case EDialogSortOrder::SortUp:   return a > b;
		}
		return false;
	}
};

struct AssetDialog
{
	struct Root {
		Path root;
		String label;
	};

public:
	const String title;
	Path currentPath;

private:
	Path navigatePath;
	TArray<Root> roots;
	TArray<Path> directories;
	TArray<AssetDialogFile> files;
	AssetDialogSorter sorter;
	bool bDirty = true;
	bool bPendingOpen = false;
	EDialogResult result = EDialogResult::None;


public:

	AssetDialog(String title)
		: title{title}
		, currentPath{ "/" }
	{}
	virtual ~AssetDialog() {}

	EDialogResult Draw();

protected:

	void OpenDialog();
	void CloseDialog(EDialogResult reason);

	virtual void DrawHeader();
	virtual void DrawContent();
	virtual void DrawFooter() {}

	virtual void OnFileSelected(const AssetDialogFile& file) {}

	void CacheCurrentDirectory();
	void CacheRoots();

	bool GetSelectedFile(AssetDialogFile& selected);

	void ResetCache() {
		bDirty = true;
		roots.Empty(false);
		directories.Empty(false);
		files.Empty(false);
	}
};
