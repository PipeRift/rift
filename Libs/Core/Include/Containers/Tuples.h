// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include <assert.h>

#include <optional>
#include <utility>


namespace Rift
{
	template <typename T1, typename T2>
	using TPair = std::pair<T1, T2>;

	template <typename... T>
	using TTuple = std::tuple<T...>;

	template <typename T>
	using TOptional = std::optional<T>;
}	 // namespace Rift
