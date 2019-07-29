// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include <nlohmann/json.hpp>
#include "Core/Strings/Name.h"
#include "Core/Reflection/ReflectionTypeTraits.h"


using Json = nlohmann::json;
DECLARE_REFLECTION_TYPE(Json);
