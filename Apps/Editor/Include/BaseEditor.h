// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <CoreObject.h>


class BaseEditor : public Rift::Object
{
	CLASS(BaseEditor, Rift::Object)

protected:
	bool bOpen = true;


public:
	BaseEditor() : Super() {}
};
