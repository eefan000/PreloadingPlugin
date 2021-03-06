﻿// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PreloadingBehaviorEditor.h"
#include "PreloadingBehaviorAssetType.h"
#include "AssetTypeCategories.h"
#include "ISettingsModule.h"
#include "PreloadingSubsystem.h"
#include "AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "FPreloadingBehaviorEditorModule"

void FPreloadingBehaviorEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type Category = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Preloading")), LOCTEXT("PreloadingAssetCategory", "Preloading"));
	TSharedRef<IAssetTypeActions> AssetTypeActions_PreloadBehavior = MakeShareable(new FAssetTypeActions_PreloadBehavior(Category));
	AssetTools.RegisterAssetTypeActions(AssetTypeActions_PreloadBehavior);

	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterSettings("Project", "Plugins", "PreloadingSubsystemSettings",
		LOCTEXT("RuntimeSettingsName", "PreloadingPlugin"),
		LOCTEXT("RuntimeSettingsDescription", "预加载框架插件"),
		GetMutableDefault<UPreloadingSubsystemSettings>()
	);
}

void FPreloadingBehaviorEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

class FTempSubsystemCollection : public FSubsystemCollectionBase
{

};
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPreloadingBehaviorEditorModule, PreloadingBehaviorEditor)