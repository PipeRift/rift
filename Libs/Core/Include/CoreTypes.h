// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "Core/Platform/Platform.h"
#include "Core/Strings/String.h"
#include "Core/Strings/Name.h"
#include "Core/Reflection/ReflectionTypeTraits.h"


/** Registry new editor-supported types here and on "EngineTypes.cpp" */

/** To registry a new native type:
 * 1. Add Definition below
 * 2. Registry on HandleHelper.cpp
 * 3. Registry widget creation on PropertyWidget.cpp
 */

DECLARE_REFLECTION_TYPE(bool);
DECLARE_REFLECTION_TYPE(u8);
DECLARE_REFLECTION_TYPE(i32);
DECLARE_REFLECTION_TYPE(u32);
DECLARE_REFLECTION_TYPE(float);

DECLARE_REFLECTION_TYPE(String);
DECLARE_REFLECTION_TYPE(Name);
