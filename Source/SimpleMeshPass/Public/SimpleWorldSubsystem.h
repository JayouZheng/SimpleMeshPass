/*********************************************************************
 *  SimpleWorldSubsystem.h
 *  Copyright (C) 2022 Jayou. All Rights Reserved.
 * 
 *  .
 *********************************************************************/

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "SimpleWorldSubsystem.generated.h"

class UTextureRenderTarget2D;
class FSimpleMeshPassManager;

UCLASS()
class USimpleWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	USimpleWorldSubsystem();

	// FTickableGameObject implementation Begin
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject implementation End

	// USubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// USubsystem implementation End

private:

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	FSimpleMeshPassManager* SimpleMeshPassManager;
};
