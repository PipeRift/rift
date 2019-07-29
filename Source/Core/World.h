// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "Assets/Scene.h"

#include "Core/Assets/AssetManager.h"
#include "ECS/ECSManager.h"
#include "Rendering/Frame.h"
#include "Rendering/Commands/RenderCommand.h"
#include "Core/Assets/AssetPtr.h"

#include "Gameplay/Components/CMesh.h"
#include "Gameplay/Components/CTransform.h"
#include "../Assets/Material.h"

enum class EWorldType : u8 {
	Editor,
	PIE,
	Standalone,
	EndingPlay
};


class World : public Object {
	CLASS(World, Object)

	TAssetPtr<Scene> scene;

	GlobalPtr<ECSManager> ecs;

	EWorldType worldType = EWorldType::Standalone;

public:

	struct Delegates {
		static Broadcast<Ptr<World>> onBeginPlay;
		static Broadcast<Ptr<World>> onEndPlay;
	};

	void Initialize();

	void BeginPlay();

	void Tick(float deltaTime);

	void EndPlay();

	void Shutdown();


	bool LoadScene(const TAssetPtr<Scene>& scene);

	virtual bool Serialize(Archive& ar) override {
		bool bResult = Super::Serialize(ar);
		bResult &= ecs->Serialize(ar);
		return bResult;
	}

	const TAssetPtr<Scene>& GetActiveScene() const { return scene; }
	TAssetPtr<Scene>&       GetActiveScene()       { return scene; }
	Ptr<ECSManager> GetECS() const { return ecs; }


	FORCEINLINE bool IsEditor() const {
#if WITH_EDITOR
		return worldType == EWorldType::Editor;
#else
		return false;
#endif
	}
	FORCEINLINE bool IsPIE() const { return worldType == EWorldType::PIE; }
	FORCEINLINE bool IsStandalone() const { return worldType == EWorldType::Standalone; }

	FORCEINLINE bool IsPlaying() const { return IsStandalone() || IsPIE(); }
};
