#pragma once

#include "AssetDialog.h"


struct SelectAssetDialog : public AssetDialog
{
	bool bShowButtons = true;
	Path selectedAsset;


	SelectAssetDialog(const String title, bool bShowButtons = true)
		: AssetDialog(title), bShowButtons{bShowButtons}
	{}

	void Open();

protected:

	virtual void DrawFooter() override;

	virtual void OnFileSelected(const AssetDialogFile& file) override {
		selectedAsset = file.path;

		if (!bShowButtons)
		{
			CloseDialog(EDialogResult::Success);
		}
	}
};
