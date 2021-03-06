#include "PreloadingSubsystem.h"
#include "UObject/UObjectHash.h"
#include "Engine/AssetManager.h"
#include "Engine/Blueprint.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "Engine/ObjectLibrary.h"

//__pragma(optimize("", off))

UPreloadingSubsystemSettings::UPreloadingSubsystemSettings(const FObjectInitializer& ObjectInitializer)
{
	PreloadingBehaviorTemplate = FSoftObjectPath(TEXT("/PreloadingPlugin/BP_PreloadingBehaviorTemplate.BP_PreloadingBehaviorTemplate"));
}

UPreloadingBehavior* UPreloadingSubsystem::GetPreloadingBehavior(TSubclassOf<UPreloadingBehavior> Class)
{
	UPreloadingBehavior** Find = BehaviorMap.Find(Class);
	return Find ? *Find : nullptr;
}

TArray<UPreloadingBehavior*> UPreloadingSubsystem::GetAllPreloadingBehavior()
{
	TArray<UPreloadingBehavior*> OutArray;
	BehaviorMap.GenerateValueArray(OutArray);
	return OutArray;
}

void UPreloadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	LoadBlueprintClass();

	// 自动注册所有UPreloadingBehavior子类
	{
		TArray<UClass*> Classes;
		GetDerivedClasses(UPreloadingBehavior::StaticClass(), Classes, true);
		for (UClass* BehaviorClass : Classes)
		{
			if (!BehaviorClass->GetName().StartsWith(TEXT("SKEL_")) && !BehaviorClass->GetName().StartsWith(TEXT("REINST_")) && !BehaviorClass->HasAllClassFlags(CLASS_Abstract))
			{
				UPreloadingBehavior*& PreloadingBehavior = BehaviorMap.Add(BehaviorClass);
				PreloadingBehavior = NewObject<UPreloadingBehavior>(this, BehaviorClass);
				// 加载永久资源
				static const FString ContextString(TEXT("UPreloadingSubsystem::Initialize -> UPreloadingBehavior::PermanentlyLoadAssets"));
				TSharedPtr<FStreamableHandle> Request = UAssetManager::GetStreamableManager().RequestAsyncLoad(PreloadingBehavior->PermanentlyLoadAssets, FStreamableDelegate(), PreloadingBehavior->Priority, true, false, ContextString);
				if (Request.IsValid())
				{
					PreloadingBehavior->PermanentlyLoadAssetsHandle = Request;
				}
			}
		}
	}
}
void UPreloadingSubsystem::LoadBlueprintClass()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked< FAssetRegistryModule >(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 加载资源目录
	AssetRegistry.ScanPathsSynchronous(GetMutableDefault<UPreloadingSubsystemSettings>()->ContentPaths);

	// 加载PreloadingBehaviorBlueprint类型
	{
		FName BaseClassName = UPreloadingBehavior::StaticClass()->GetFName();

		FARFilter Filter;
		Filter.ClassNames.Add(UPreloadingBehaviorBlueprint::StaticClass()->GetFName());
		for (auto& ContentPath : GetMutableDefault<UPreloadingSubsystemSettings>()->ContentPaths)
		{
			Filter.PackagePaths.Add(FName(*ContentPath));
		}
		Filter.bRecursiveClasses = true;
		Filter.bRecursivePaths = true;

		TArray< FAssetData > AssetList;
		AssetRegistry.GetAssets(Filter, AssetList);

		// 遍历所有PreloadingBehaviorBlueprint类型资产
		for (auto const& Asset : AssetList)
		{
			// 获取通过蓝图生成出来的类的路径
			const FAssetDataTagMapSharedView::FFindTagResult GeneratedClassValue = Asset.TagsAndValues.FindTag(TEXT("GeneratedClass"));
			if (GeneratedClassValue.IsSet())
			{
				// 加载蓝图对象
				TSoftClassPtr< UObject >(FStringAssetReference(FPackageName::ExportTextPathToObjectPath(GeneratedClassValue.GetValue()))).LoadSynchronous();
			}
		}
	}
}
//__pragma(optimize("", on))