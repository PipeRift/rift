// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"

#include "../Events/Broadcast.h"
#include "Keys.h"


using KeyBroadcast = Broadcast<EKeyPressState>;
using KeyPressedBroadcast = Broadcast<>;
using AxisBroadcast = Broadcast<float>;


struct TriggerAction {
	struct KeyBinding {
		EKey key;
		EKeyModifier mods;
	};

	Name name;
	TArray<KeyBinding> bindings;

private:

	KeyBroadcast ev;


public:

	TriggerAction(Name name = Name::None()) : name(name), bindings{}, ev{} {}

	KeyBroadcast& OnUpdate() { return ev; }
};


struct AxisAction {
	struct KeyBinding {
		EKey key;
		EKeyModifier mods;
		float multiplier = 1.f;
	};

	struct AxisBinding {
		EAxis axis;
		EKeyModifier mods;
		float multiplier = 1.f;
	};

	Name name;
	TArray<AxisBinding> axisBindings;
	TArray<KeyBinding> keyBindings;

	float lastFrameValue;

private:

	AxisBroadcast ev;


public:

	AxisAction(Name name = Name::None()) : name(name), axisBindings{}, keyBindings{}, lastFrameValue{0.f}, ev{} {}

	AxisBroadcast& OnUpdate() { return ev; }
};
