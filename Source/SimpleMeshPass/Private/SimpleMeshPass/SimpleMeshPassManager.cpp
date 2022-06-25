/*********************************************************************
 *  SimpleMeshPassManager.cpp
 *  Copyright (C) 2022 Jayou. All Rights Reserved.
 * 
 *  .
 *********************************************************************/

#include "SimpleMeshPassManager.h"
#include "EngineModule.h"
#include "TextureResource.h"
#include "RenderGraph.h"
#include "Renderer/Private/ScenePrivate.h"
#include "Renderer/Private/SceneRendering.h"
#include "SimpleMeshDrawCommandPass.h"
#include "SimpleMeshPassProcessor.h"
#include "SimpleMeshPassCommon.h"

FSimpleMeshPassManager::FSimpleMeshPassManager()
	: RenderTargetResource(nullptr)
{
	RenderHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(FPostOpaqueRenderDelegate::CreateRaw(this, &FSimpleMeshPassManager::Render));
}

FSimpleMeshPassManager::~FSimpleMeshPassManager()
{
	RenderTargetResource = nullptr;

	GetRendererModule().RemovePostOpaqueRenderDelegate(RenderHandle);
	RenderHandle.Reset();
}

void FSimpleMeshPassManager::SetRenderTarget(FTextureRenderTargetResource* InRenderTargetResource)
{
	RenderTargetResource = InRenderTargetResource;
}

FInt32Range GetDynamicMeshElementRange(const FViewInfo& View, uint32 PrimitiveIndex)
{
	int32 Start = 0;	// inclusive
	int32 AfterEnd = 0;	// exclusive

	// DynamicMeshEndIndices contains valid values only for visible primitives with bDynamicRelevance.
	if (View.PrimitiveVisibilityMap[PrimitiveIndex])
	{
		const FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveIndex];
		if (ViewRelevance.bDynamicRelevance)
		{
			Start = (PrimitiveIndex == 0) ? 0 : View.DynamicMeshEndIndices[PrimitiveIndex - 1];
			AfterEnd = View.DynamicMeshEndIndices[PrimitiveIndex];
		}
	}

	return FInt32Range(Start, AfterEnd);
}

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FSimpleMeshPassUniformParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColor)
	SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT(FSimpleMeshPassUniformParameters, "SimpleMeshPass", SceneTextures);

BEGIN_SHADER_PARAMETER_STRUCT(FSimpleMeshPassParameters, )
	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
	SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSimpleMeshPassUniformParameters, PassUniformBuffer)
	SHADER_PARAMETER_STRUCT_INCLUDE(FInstanceCullingDrawParams, InstanceCullingDrawParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

void FSimpleMeshPassManager::Render(FPostOpaqueRenderParameters& InParameters)
{
	FRDGBuilder& GraphBuilder = *InParameters.GraphBuilder;
	const FViewInfo& View = *InParameters.View;
	
	FScene* Scene = nullptr;
	if (View.Family->Scene != nullptr)
	{
		Scene = static_cast<FScene*>(View.Family->Scene);		
	}
	else
	{
		UE_LOG(LogSimpleMeshPass, Error, TEXT("View.Family->Scene is NULL! FSimpleMeshPassManager::Render() will not run!"));
		return;
	}
	
	if (RenderTargetResource == nullptr || RenderTargetResource->GetTexture2DRHI() == nullptr)
	{
		UE_LOG(LogSimpleMeshPass, Error, TEXT("FSimpleMeshPassManager::RenderTargetResource or TextureRHI is NULL! FSimpleMeshPassManager::Render() will not run!"));
		return;
	}

	FSimpleMeshDrawCommandPass* SimpleMeshPass = GraphBuilder.AllocObject<FSimpleMeshDrawCommandPass>(View, nullptr);

	FMeshPassProcessorRenderState DrawRenderState;
	DrawRenderState.SetDepthStencilState(TStaticDepthStencilState<true, CF_DepthNearOrEqual>().GetRHI());

	FSimpleMeshPassMeshProcessor MeshProcessor(
		Scene,
		&View,
		DrawRenderState,
		SimpleMeshPass->GetDynamicPassMeshDrawListContext());

	// Gather MeshBatch from View.DynamicMeshElements.
	//for (int32 MeshBatchIndex = 0; MeshBatchIndex < View.DynamicMeshElements.Num(); MeshBatchIndex++)
	//{
	//	const FMeshBatchAndRelevance& MeshAndRelevance = View.DynamicMeshElements[MeshBatchIndex];
	//	const uint64 BatchElementMask = ~0ull;
	//	MeshProcessor.AddMeshBatch(*MeshAndRelevance.Mesh, BatchElementMask, MeshAndRelevance.PrimitiveSceneProxy);
	//}

	// Gather MeshBatch from Scene->Primitives.
	for (int32 PrimitiveIndex = 0; PrimitiveIndex < Scene->Primitives.Num(); PrimitiveIndex++)
	{
		const FPrimitiveSceneInfo* PrimitiveSceneInfo = Scene->Primitives[PrimitiveIndex];

		if (View.PrimitiveVisibilityMap[PrimitiveSceneInfo->GetIndex()])
		{
			const FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap[PrimitiveSceneInfo->GetIndex()];

			if (ViewRelevance.bRenderInMainPass && ViewRelevance.bStaticRelevance)
			{
				for (int32 StaticMeshIdx = 0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++)
				{
					const FStaticMeshBatch& StaticMesh = PrimitiveSceneInfo->StaticMeshes[StaticMeshIdx];

					if (View.StaticMeshVisibilityMap[StaticMesh.Id])
					{
						const uint64 DefaultBatchElementMask = ~0ul;
						MeshProcessor.AddMeshBatch(StaticMesh, DefaultBatchElementMask, StaticMesh.PrimitiveSceneInfo->Proxy);
					}
				}
			}

			if (ViewRelevance.bRenderInMainPass && ViewRelevance.bDynamicRelevance)
			{
				const FInt32Range MeshBatchRange = GetDynamicMeshElementRange(View, PrimitiveSceneInfo->GetIndex());

				for (int32 MeshBatchIndex = MeshBatchRange.GetLowerBoundValue(); MeshBatchIndex < MeshBatchRange.GetUpperBoundValue(); ++MeshBatchIndex)
				{
					const FMeshBatchAndRelevance& MeshAndRelevance = View.DynamicMeshElements[MeshBatchIndex];
					const uint64 BatchElementMask = ~0ull;

					MeshProcessor.AddMeshBatch(*MeshAndRelevance.Mesh, BatchElementMask, MeshAndRelevance.PrimitiveSceneProxy);
				}
			}
		}
	}

	FRDGTextureRef RenderTarget = RegisterExternalTexture(GraphBuilder, RenderTargetResource->GetTexture2DRHI(), TEXT("SimpleMeshPassRT"));
	AddClearRenderTargetPass(GraphBuilder, RenderTarget, FLinearColor::Green);

	FSimpleMeshPassUniformParameters* SimpleMeshPassParameters = GraphBuilder.AllocParameters<FSimpleMeshPassUniformParameters>();
	SimpleMeshPassParameters->SceneColor = InParameters.ColorTexture;
	SimpleMeshPassParameters->SceneColorSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();

	FSimpleMeshPassParameters* PassParameters = GraphBuilder.AllocParameters<FSimpleMeshPassParameters>();
	PassParameters->View = View.ViewUniformBuffer;
	PassParameters->PassUniformBuffer = GraphBuilder.CreateUniformBuffer(SimpleMeshPassParameters);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTarget, ERenderTargetLoadAction::ELoad);
	PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(InParameters.DepthTexture, ERenderTargetLoadAction::ELoad, FExclusiveDepthStencil::DepthRead_StencilNop);

	SimpleMeshPass->BuildRenderingCommands(GraphBuilder, View, Scene->GPUScene, PassParameters->InstanceCullingDrawParams);

	FIntRect ViewportRect = InParameters.ViewportRect;
	FIntRect ScissorRect = FIntRect(FIntPoint(EForceInit::ForceInitToZero), RenderTarget->Desc.Extent);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("SimpleMeshPass"),
		PassParameters,
		ERDGPassFlags::Raster | ERDGPassFlags::NeverCull,
		[this, ViewportRect, ScissorRect, SimpleMeshPass, PassParameters](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(ViewportRect.Min.X, ViewportRect.Min.Y, 0.0f, ViewportRect.Max.X, ViewportRect.Max.Y, 1.0f);

			RHICmdList.SetScissorRect(
				true, 
				ScissorRect.Min.X >= ViewportRect.Min.X ? ScissorRect.Min.X : ViewportRect.Min.X,
				ScissorRect.Min.Y >= ViewportRect.Min.Y ? ScissorRect.Min.Y : ViewportRect.Min.Y, 
				ScissorRect.Max.X <= ViewportRect.Max.X ? ScissorRect.Max.X : ViewportRect.Max.X,
				ScissorRect.Max.Y <= ViewportRect.Max.Y ? ScissorRect.Max.Y : ViewportRect.Max.Y);

			SimpleMeshPass->SubmitDraw(RHICmdList, PassParameters->InstanceCullingDrawParams);
		});
}
