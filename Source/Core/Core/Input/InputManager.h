// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"

#include "../Events/Broadcast.h"
#include "Keys.h"
#include "Actions.h"


class InputManager : public Object {
	CLASS(InputManager, Object)

private:

	/** BEGIN INPUT STATE */

	/** Keeps the state of an axis */
	struct AxisStates {
		TArray<float> values;

		AxisStates() : values{ (i32)EAxis::Max } {}

		float& operator[](EAxis key) { return values[(i32)key]; }
		const float& operator[](EAxis key) const { return values[(i32)key]; }
	};

	/** Keeps the state of a key */
	struct KeyStates {
		TArray<EKeyPressState> states;

		KeyStates() : states{ (i32)EKey::Max } {}

		EKeyPressState& operator[](EKey key) { return states[i32(key)]; }
		const EKeyPressState& operator[](EKey key) const { return states[i32(key)]; }
	};

	/** Keeps the state of a modifier. TODO: Calculate mods flag */
	struct ModStates {
		EKeyModifier flags;

		void Set(EKeyModifier mod, EKeyPressState state);

		/** @return true if this modifier is active */
		bool operator[](EKeyModifier mod) const;
	};

	AxisStates axisStates;
	KeyStates keyStates;
	ModStates modStates;
	/** END INPUT STATE */


	/** Actions allow complex flag key combinations. E.g: W | UpArrow */
	mutable TArray<TriggerAction> triggerActions;
	mutable TArray<AxisAction>    axisActions;

	KeyBroadcast onKey;
	KeyPressedBroadcast onKeyPressed;

public:

	bool Tick(float deltaTime, Ptr<class UIManager> ui, u32 windowId);

private:

	void UpdatePressedKeys();
	void UpdatePressedMods();
	void ResetAxis();

	void UpdateKey(EKey key, EKeyPressState state);

	void UpdateAxis(EAxis axis, float value);

	void NotifyAllAxis();
	void NotifyAllKeys();

public:

	FORCEINLINE EKeyPressState GetKeyState(EKey key) const { return keyStates[key]; }
	FORCEINLINE bool IsKeyPressed(EKey key) const { return keyStates[key] == EKeyPressState::Pressed; }
	FORCEINLINE bool IsModifierPressed(EKeyModifier mod) const { return modStates[mod]; }


	/** EVENTS */

	KeyBroadcast& CreateTriggerAction(Name actionName, TArray<TriggerAction::KeyBinding>&& bindings) const;
	KeyBroadcast& CreateTriggerAction(Name actionName, TriggerAction::KeyBinding&& binding) const {
		return CreateTriggerAction(actionName, TArray<TriggerAction::KeyBinding>{ MoveTemp(binding) });
	}
	KeyBroadcast* FindTriggerAction(Name actionName) const;

	AxisBroadcast& CreateAxisAction(Name actionName, TArray<AxisAction::KeyBinding>&& keyBindings, TArray<AxisAction::AxisBinding>&& axisBindings) const;
	AxisBroadcast* FindAxisAction(Name actionName) const;
};
