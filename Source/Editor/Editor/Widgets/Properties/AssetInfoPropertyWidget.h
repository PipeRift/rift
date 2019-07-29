// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "../PropertyWidget.h"

#include "Core/Reflection/Runtime/TPropertyHandle.h"
#include "Core/Assets/AssetInfo.h"
#include "Core/Assets/AssetPtr.h"


/** Widget for editing AssetInfos and AssetPtrs */
class AssetInfoPropertyWidget : public PropertyWidget {
	CLASS(AssetInfoPropertyWidget, PropertyWidget)

protected:

	virtual void Tick(float) override;

	TPropertyHandle<AssetInfo>* GetInfoHandle() const {
		return dynamic_cast<TPropertyHandle<AssetInfo>*>(prop.get());
	}

	BaseAssetPtr* GetAssetPtrValuePtr() const {
		return static_cast<BaseAssetPtr*>(prop->GetRawValuePtr());
	}

private:

	bool EditAssetInfo(AssetInfo& info);
};

#endif
