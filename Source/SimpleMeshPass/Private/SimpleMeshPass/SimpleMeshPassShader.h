/*********************************************************************
 *  SimpleMeshPassShader.h.h
 *  Copyright (C) 2022 Jayou. All Rights Reserved.
 * 
 *  .
 *********************************************************************/

#pragma once

#include "MeshMaterialShader.h"
#include "MeshPassProcessor.h"

class FScene;

class FSimpleMeshPassShaderElementData : public FMeshMaterialShaderElementData
{
public:
	float ParameterValue;
};

class FSimpleMeshPassVS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FSimpleMeshPassVS, MeshMaterial);

public:

	FSimpleMeshPassVS() = default;
	FSimpleMeshPassVS(const ShaderMetaType::CompiledShaderInitializerType & Initializer) :
		FMeshMaterialShader(Initializer)
	{
		// Per Mesh Update Parameter Bind.
		//Parameter.Bind(Initializer.ParameterMap, TEXT("Parameter"));
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// Set Define in Shader. 
		//OutEnvironment.SetDefine(TEXT("Define"), Value);
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) &&
			Parameters.MaterialParameters.bIsDefaultMaterial && 
			(Parameters.VertexFactoryType->GetFName() == FName(TEXT("FLocalVertexFactory")) || 
				Parameters.VertexFactoryType->GetFName() == FName(TEXT("TGPUSkinVertexFactoryDefault")));
	}

	void GetShaderBindings(
		const FScene * Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FSimpleMeshPassShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);

		// Assign Value to Parameter.
		//ShaderBindings.Add(Parameter, ShaderElementData.ParameterValue);
	}

private:

	// Per Mesh Update Parameter.
	//LAYOUT_FIELD(FShaderParameter, Parameter);
};

class FSimpleMeshPassPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FSimpleMeshPassPS, MeshMaterial);

public:

	FSimpleMeshPassPS() = default;
	FSimpleMeshPassPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FMeshMaterialShader(Initializer)
	{
		// Per Mesh Update Parameter Bind.
		Parameter.Bind(Initializer.ParameterMap, TEXT("Parameter"));
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		// Set Define in Shader. 
		//OutEnvironment.SetDefine(TEXT("Define"), Value);
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		// Try to Reduce Shader Permutation.
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) &&
			Parameters.MaterialParameters.bIsDefaultMaterial &&
			(Parameters.VertexFactoryType->GetFName() == FName(TEXT("FLocalVertexFactory")) ||
				Parameters.VertexFactoryType->GetFName() == FName(TEXT("TGPUSkinVertexFactoryDefault")));
	}

	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FSimpleMeshPassShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);

		// Assign Value to Parameter.
		//ShaderBindings.Add(Parameter, ShaderElementData.ParameterValue);

		FVector4f Color;
		TArray<float> CustomData = PrimitiveSceneProxy->GetCustomPrimitiveData()->Data;

		for (int32 Index = 0; Index < 3; Index++)
		{
			Color[Index] = CustomData.IsValidIndex(Index) ? CustomData[Index] : 1.0f;
		}

		Color[3] = 1.0f;
		ShaderBindings.Add(Parameter, Color);
	}

private:

	// Per Mesh Update Parameter.
	LAYOUT_FIELD(FShaderParameter, Parameter);
};
