// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleMeshPass.h"
#include "SimpleMeshPassCommon.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FSimpleMeshPassModule"

DEFINE_LOG_CATEGORY(LogSimpleMeshPass);

void FSimpleMeshPassModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Maps virtual shader source directory /Plugin/GPULightmass to the plugin's actual Shaders directory.
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("SimpleMeshPass"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/SimpleMeshPass"), PluginShaderDir);
}

void FSimpleMeshPassModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleMeshPassModule, SimpleMeshPass)