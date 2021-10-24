// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Entt/PolyStorage.h"


template<typename Entity>
struct entt::poly_storage_traits<Entity>
{
	using storage_type = entt::poly<PolyStorage<Entity>>;
};
