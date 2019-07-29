// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include <EASTL/type_traits.h>


/** CLASS TRAITS */

/** Default traits go here */
template <typename T>
struct BaseClassTraits { enum {
	HasCustomSerialize = false,
	HasGlobalSerialize = false,
	HasDetailsWidget = false,
	HasPostSerialize = false
};};

/** Custom traits go here */
#define DEFINE_CLASS_TRAITS(Class, ...)\
template <>\
struct ClassTraits<Class> : public BaseClassTraits<Class> {\
	enum {__VA_ARGS__};\
}
#define DEFINE_TEMPLATE_CLASS_TRAITS(Class, ...)\
template <typename T>\
struct ClassTraits<Class<T>> : public BaseClassTraits<Class<T>> {\
	enum {__VA_ARGS__};\
}

template <typename T>
struct ClassTraits : public BaseClassTraits<T> {};
