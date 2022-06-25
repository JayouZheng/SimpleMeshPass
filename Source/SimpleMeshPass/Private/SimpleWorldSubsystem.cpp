/*********************************************************************
 *  SimpleWorldSubsystem.cpp
 *  Copyright (C) 2022 Jayou. All Rights Reserved.
 * 
 *  .
 *********************************************************************/

#pragma optimize("", off)

#include "SimpleWorldSubsystem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RHICommandList.h"
#include "SimpleMeshPass/SimpleMeshPassManager.h"
#include "SimpleMeshPassCommon.h"
#include "Kismet/KismetRenderingLibrary.h"

USimpleWorldSubsystem::USimpleWorldSubsystem()
{
	RenderTarget = nullptr;
}

void USimpleWorldSubsystem::Tick(float DeltaTime)
{
	if (SimpleMeshPassManager != nullptr && IsValid(RenderTarget))
	{
		// Update Before Render. Or Resize of RenderTagrt will Crash.
		ENQUEUE_RENDER_COMMAND(UpdateRenderTargetCmd)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				SimpleMeshPassManager->SetRenderTarget(RenderTarget->GetRenderTargetResource());
			});
	}
}

TStatId USimpleWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USimpleWorldSubsystem, STATGROUP_Tickables);
}

void USimpleWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString RenderTargetPath = TEXT("/SimpleMeshPass/RenderTargets/RT_RenderTarget2D.RT_RenderTarget2D");
	RenderTarget = LoadObject<UTextureRenderTarget2D>(nullptr, *RenderTargetPath);
	if (IsValid(RenderTarget))
	{
		ENQUEUE_RENDER_COMMAND(CreateSimpleMeshPassManagerCmd)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				SimpleMeshPassManager = new FSimpleMeshPassManager;
				SimpleMeshPassManager->SetRenderTarget(RenderTarget->GetRenderTargetResource());
			});

		UE_LOG(LogSimpleMeshPass, Log, TEXT("Expected UTextureRenderTarget2D successfully loaded! Path: %s"), *RenderTargetPath);
	}
	else
	{
		UE_LOG(LogSimpleMeshPass, Warning, TEXT("Expected UTextureRenderTarget2D does not exit! Path: %s"), *RenderTargetPath);
	}
}

void USimpleWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	//UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, FLinearColor(0, 0, 0, 1));

	ENQUEUE_RENDER_COMMAND(DeleteSimpleMeshPassManagerCmd)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			if (SimpleMeshPassManager != nullptr)
			{
				delete SimpleMeshPassManager;
				SimpleMeshPassManager = nullptr;
			}
		});
}

#pragma optimize("", on)
