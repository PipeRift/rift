// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include <assert.h>
#include <EASTL/utility.h>

template <typename T1, typename T2>
using TPair = eastl::pair<T1, T2>;

template <typename... T>
using TTuple = eastl::tuple<T...>;
