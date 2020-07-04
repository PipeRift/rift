// Copyright 2015-2020 Piperift - All rights reserved
#include "Assets/AssetPtr.h"

#if WITH_EDITOR

#	include "UI/Widgets/Properties/AssetInfoPropertyWidget.h"

Class* BaseAssetPtr::GetDetailsWidgetClass()
{
	return AssetInfoPropertyWidget::StaticClass();
}

#endif