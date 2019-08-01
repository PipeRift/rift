#pragma once

#include "AssetDialog.h"


struct NewAssetDialog : public AssetDialog
{
	String newAssetName;
	Path finalPath;

	NewAssetDialog(const String title)
		: AssetDialog(title)
	{}

	void Open();

protected:

	virtual void DrawFooter() override;

	virtual void OnFileSelected(const AssetDialogFile& file) override{
		newAssetName = FileSystem::ToString(file.path.filename());
	}
};
