// © 2019 Miguel Fernández Arce - All rights reserved

#include "Assets/AssetPtr.h"

#if WITH_EDITOR

#	include "UI/Widgets/Properties/AssetInfoPropertyWidget.h"

Class* BaseAssetPtr::GetDetailsWidgetClass()
{
	return AssetInfoPropertyWidget::StaticClass();
}

#endif