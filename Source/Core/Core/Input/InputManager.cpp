// Copyright 2015-2019 Piperift - All rights reserved

#include "InputManager.h"
#include <SDL_events.h>
#if WITH_EDITOR
#include <imgui/imgui.h>
#endif

#include "Rendering/Renderer.h"
#include "UI/UIManager.h"
#include "Tools/Profiler.h"


bool InputManager::Tick(float deltaTime, Ptr<UIManager> ui, u32 windowId)
{
	ScopedGameZone("Input");

	// Mark all axis not dirty

#if WITH_EDITOR
	auto& io = ImGui::GetIO();
	const bool bUICapturedMouse    = io.WantCaptureMouse;
	const bool bUICapturedKeyboard = io.WantCaptureKeyboard;
#endif

	UpdatePressedKeys();
	ResetAxis();

	bool bFinish = false;
	SDL_PumpEvents();
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ui->OnSDLEvent(&event);

		// Window events
		switch (event.type)
		{
		case SDL_WINDOWEVENT:
			if (event.window.event != SDL_WINDOWEVENT_CLOSE || event.window.windowID != windowId)
				break;
		case SDL_QUIT:
			bFinish = true;
			break;
		}

		// Mouse events

#if WITH_EDITOR
		// NOTE: Disabled for causing locking issues in editor. We don't really need it yet
		//if (!bUICapturedMouse)
#endif
		{
			switch (event.type)
			{
			case SDL_MOUSEMOTION: {
				v2_i32 pos{};
				SDL_GetRelativeMouseState(&pos.x, &pos.y);
				if (Math::Abs(pos.x) > 0)
					UpdateAxis(EAxis::MouseX, (float)pos.x);
				if (Math::Abs(pos.y) > 0)
					UpdateAxis(EAxis::MouseY, (float)pos.y);
				break;
			}
			case SDL_MOUSEWHEEL:
				if (Math::Abs(event.wheel.x) > 0)
					UpdateAxis(EAxis::MouseWheelX, (float)event.wheel.x);
				if (Math::Abs(event.wheel.y) > 0)
					UpdateAxis(EAxis::MouseWheelY, (float)event.wheel.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					UpdateKey(EKey::MouseLeft, EKeyPressState::Press);
				if (event.button.button == SDL_BUTTON_RIGHT)
					UpdateKey(EKey::MouseRight, EKeyPressState::Press);
				if (event.button.button == SDL_BUTTON_MIDDLE)
					UpdateKey(EKey::MouseCenter, EKeyPressState::Press);
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					UpdateKey(EKey::MouseLeft, EKeyPressState::Release);
				if (event.button.button == SDL_BUTTON_RIGHT)
					UpdateKey(EKey::MouseRight, EKeyPressState::Release);
				if (event.button.button == SDL_BUTTON_MIDDLE)
					UpdateKey(EKey::MouseCenter, EKeyPressState::Release);
				break;
			}
		}

		// Keyboard events
#if WITH_EDITOR
		if (!bUICapturedKeyboard)
#endif
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				EKeyPressState state = (event.type == SDL_KEYDOWN) ? EKeyPressState::Press : EKeyPressState::Release;
				const SDL_Keymod sdlModState = SDL_GetModState();

				// Automatic casting to Rift keys
				const auto key = static_cast<EKey>(event.key.keysym.scancode);

				// Update modifiers
				switch (key)
				{
				case EKey::LShift:
					modStates.Set(EKeyModifier::LShift, state);
					break;
				case EKey::RShift:
					modStates.Set(EKeyModifier::RShift, state);
					break;
				case EKey::LCtrl:
					modStates.Set(EKeyModifier::LCtrl, state);
					break;
				case EKey::RCtrl:
					modStates.Set(EKeyModifier::RCtrl, state);
					break;
				case EKey::LAlt:
					modStates.Set(EKeyModifier::LAlt, state);
					break;
				case EKey::RAlt:
					modStates.Set(EKeyModifier::RAlt, state);
					break;
				};
				UpdateKey(key, state);
				break;
			}
		}
	}

	NotifyAllAxis();
	NotifyAllKeys();

	return bFinish;
}

void InputManager::UpdatePressedKeys()
{
	for (auto& state : keyStates.states)
	{
		switch (state)
		{
		case EKeyPressState::Press:
			state = EKeyPressState::Pressed;
			break;
		case EKeyPressState::Release:
			state = EKeyPressState::Released;
			break;
		}
	}
}

void InputManager::UpdatePressedMods()
{
	modStates.flags = EKeyModifier(0);
}

void InputManager::ResetAxis()
{
	// Efficiently set all values to 0
	axisStates.values.Empty(false);
	axisStates.values.Assign((i32)EAxis::Max, 0.f);
}

void InputManager::UpdateKey(EKey key, EKeyPressState state)
{
	EKeyPressState& currentState = keyStates[key];

	// Cant set press/release twice
	if ((state == EKeyPressState::Press   && currentState != EKeyPressState::Pressed) ||
		(state == EKeyPressState::Release && currentState != EKeyPressState::Released))
	{
		currentState = state;
	}
}

void InputManager::UpdateAxis(EAxis axis, float value)
{
	axisStates[axis] = value;
}

void InputManager::NotifyAllAxis()
{
	for (auto& action : axisActions) {
		float value = 0.0f;

		for (const auto& keyBinding : action.keyBindings)
		{
			if (keyStates[keyBinding.key] != EKeyPressState::Released)
			{
				value += keyBinding.multiplier;
			}
		}

		for (const auto& axisBinding : action.axisBindings)
		{
			value += axisStates[axisBinding.axis] * axisBinding.multiplier;
		}

		// If value is not close to 0
		if (Math::Abs(value) > Math::SMALL_NUMBER)
		{
			action.OnUpdate().DoBroadcast(value);
			action.lastFrameValue = value;
		}
		/*else if (action.lastFrameValue != 0.f)
		{
			// Ensure value ends up being pure 0
			action.OnUpdate().DoBroadcast(0.f);
			action.lastFrameValue = 0.f;
		}*/
	}
}

void InputManager::NotifyAllKeys()
{
	for (auto& action : triggerActions) {
		EKeyPressState state{ EKeyPressState::Released };

		for (const auto& binding : action.bindings)
		{
			state = Math::Max(state, keyStates[binding.key]);
		}

		if (state != EKeyPressState::Released)
		{
			action.OnUpdate().DoBroadcast(state);
		}
	}
}

KeyBroadcast& InputManager::CreateTriggerAction(Name actionName, TArray<TriggerAction::KeyBinding>&& bindings) const
{
	if (actionName.IsNone())
	{
		actionName = { "Invalid" };
	}

	TriggerAction* currentAction = triggerActions.Find([actionName](const TriggerAction& action) {
		return action.name == actionName;
	});

	if (!currentAction)
	{
		triggerActions.Add({ actionName });
		currentAction = &triggerActions.Last();
	}

	currentAction->bindings = eastl::move(bindings);

	return currentAction->OnUpdate();
}

KeyBroadcast* InputManager::FindTriggerAction(Name actionName) const
{
	if (actionName.IsNone())
		return nullptr;

	auto* foundAction = triggerActions.Find([actionName](const TriggerAction& action) {
		return action.name == actionName;
	});

	return foundAction? &foundAction->OnUpdate() : nullptr;
}

AxisBroadcast& InputManager::CreateAxisAction(Name actionName, TArray<AxisAction::KeyBinding>&& keyBindings, TArray<AxisAction::AxisBinding>&& axisBindings) const
{
	if (actionName.IsNone())
		actionName = { "Invalid" };

	AxisAction* currentAction = axisActions.Find([actionName](const AxisAction& action) {
		return action.name == actionName;
	});

	if (!currentAction)
	{
		axisActions.Add({ actionName });
		currentAction = &axisActions.Last();
	}

	currentAction->keyBindings = eastl::move(keyBindings);
	currentAction->axisBindings = eastl::move(axisBindings);

	return currentAction->OnUpdate();
}

AxisBroadcast* InputManager::FindAxisAction(Name actionName) const
{
	if (actionName.IsNone())
		return nullptr;

	auto* foundAction = axisActions.Find([actionName](const AxisAction& action) {
		return action.name == actionName;
	});
	return foundAction ? &foundAction->OnUpdate() : nullptr;
}

void InputManager::ModStates::Set(EKeyModifier mod, EKeyPressState state)
{
	switch (state)
	{
	case EKeyPressState::Press:
		flags |= mod;
		break;
	case EKeyPressState::Release:
		flags &= ~mod;
		break;
	}
}

bool InputManager::ModStates::operator[](EKeyModifier mod) const
{
	return (flags & mod) > 0;
}
