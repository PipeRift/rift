// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "CoreTypes.h"


struct EventHandle {
private:

	static u64 counter;

	static u64 NewId() {
		if (counter == 0)
			++counter;
		return counter++;
	}

	u64 id;

	/** Used for invalidation */
	EventHandle(u64 customId) : id(customId) {}

public:

	static EventHandle Invalid() { return { 0 }; }

	EventHandle() : id(NewId()) {}

	u64 Id() const { return id; }

	bool IsValid() const { return id != 0; }
	operator bool() const { return IsValid(); }
};
