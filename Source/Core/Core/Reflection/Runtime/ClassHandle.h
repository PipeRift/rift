// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Core/Reflection/Class.h"
#include "TPropertyHandle.h"


class ClassHandle {

	const Ptr<BaseObject> instance;

public:

	ClassHandle() = delete;
	ClassHandle(ClassHandle&&) = default;
	ClassHandle(const ClassHandle&) = default;
	ClassHandle& operator=(ClassHandle&&) = default;
	ClassHandle& operator=(const ClassHandle&) = default;
	virtual ~ClassHandle() = default;

	ClassHandle(const Ptr<BaseObject>& instance) : instance(instance) {}
};
