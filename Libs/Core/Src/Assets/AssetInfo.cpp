// © 2019 Miguel Fernández Arce - All rights reserved

#include "Assets/AssetInfo.h"

#include "Serialization/Archive.h"


#if WITH_EDITOR
#	include "UI/Widgets/Properties/AssetInfoPropertyWidget.h"
#endif


bool AssetInfo::Serialize(class Archive& ar, const char* name)
{
	ar(name, id);
	return true;
}

#if WITH_EDITOR

Class* AssetInfo::GetDetailsWidgetClass()
{
	return AssetInfoPropertyWidget::StaticClass();
}

#endif
