/*********************************************************************
 *  SimpleMeshPassManager.h
 *  Copyright (C) 2022 Jayou. All Rights Reserved.
 * 
 *  .
 *********************************************************************/

#pragma once

#include "RendererInterface.h"
#include "RenderTargetPool.h"

class FTextureRenderTargetResource;

class FSimpleMeshPassManager
{
public:

	FSimpleMeshPassManager();
	~FSimpleMeshPassManager();

	void CacheScene(const FSceneInterface* InScene);
	void SetRenderTarget(FTextureRenderTargetResource* InRenderTargetResource);
	void Render(FPostOpaqueRenderParameters& InParameters);

private:

	FDelegateHandle RenderHandle;
	
	const FTextureRenderTargetResource* RenderTargetResource;
};
