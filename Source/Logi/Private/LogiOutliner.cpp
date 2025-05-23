#include "LogiOutliner.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Engine/StaticMeshActor.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/BlueprintSupport.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"
#include "Utils/LogiUtils.h"


namespace Logi::LogiOutliner
{
	// Main method, that adds everything to the Outliner 
    void AddLogiLogicToOutliner(UWorld* World, bool& bSuccess, FString& StatusMessage)
    {
        AddThermalCameraToOutliner(World);
        AddThermalPostProcessVolumeToOutliner(World, bSuccess, StatusMessage);
    }


    void AddThermalCameraToOutliner(UWorld* World)
    {
        const FString BlueprintPath = "/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController";

        const FString BlueprintLabel = TEXT("BP_Logi_ThermalController");

        UObject* BlueprintObject = StaticLoadObject(UObject::StaticClass(), nullptr, *BlueprintPath);
        
        if (!BlueprintObject) {
            UE_LOG(LogTemp, Error, TEXT("Could not load blueprint: %s"), *BlueprintPath);
            return;  
        }

        const UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintObject);

        // Check if blueprint with the given label already exists in the Outliner/scene 
        AActor* ExistingOutlinerBlueprint = FindActorInOutlinerByLabel(World, BlueprintLabel);
        
        if (ExistingOutlinerBlueprint) {
            // If it exists, asks if user wants to overwrite
            if (OverwritePopup(BlueprintLabel)) {

                // User wishes to overwrite, so we delete the existing one and spawn/add the blueprint

                DeleteActorInOutliner(ExistingOutlinerBlueprint);
                SpawnNewBlueprint(World, Blueprint);
            }
        }
        else {
            // If blueprint does not exist, spawn/add the blueprint
            SpawnNewBlueprint(World, Blueprint);
        }

    }


    // Show overwrite pop-up
    bool OverwritePopup(const FString& NameOfFile)
    {
        // Pop-up Yes/No alternatives
        
        const FText DialogText = FText::Format(FText::FromString(TEXT("The file '{0}' already exists in the scene. Do you want to overwrite it?")), FText::FromString(NameOfFile));
        const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogText);

        return Response == EAppReturnType::Yes; // If user chooses "Yes", return true
    }

    // Find Actor with the given label in the Outliners Logi_ThermalCamera folder. Returns the actor if existing, returns nullptr if not.
    AActor* FindActorInOutlinerByLabel(const UWorld* World, const FString& ActorLabel)
    {
        const FString TargetFolder = TEXT("Logi_ThermalCamera");

        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;

            const FString FolderPath = Actor->GetFolderPath().ToString();
            const FString Label = Actor->GetActorLabel();

            if (FolderPath.Equals(TargetFolder) && Label.Equals(ActorLabel))
            {
                UE_LOG(LogTemp, Log, TEXT("Found existing actor: %s in folder %s"), *ActorLabel, *TargetFolder);
                return Actor;
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Actor %s does not exist in folder %s"), *ActorLabel, *TargetFolder);
        return nullptr;
    }


    // Delete existing actor from Outliner (NOT any file content browser - but in Outliner/Scene)
    void DeleteActorInOutliner(AActor* Actor)
    {
        if (Actor)
        {
            UE_LOG(LogTemp, Log, TEXT("Deleting actor: %s"), *Actor->GetActorLabel());
            Actor->Destroy();
        }
    }


    // Spawn/add given blueprint to Outliner(Scene)
    void SpawnNewBlueprint(UWorld* World, const UBlueprint* Blueprint)
    {
        if (Blueprint && Blueprint->GeneratedClass) {
            
            const FActorSpawnParameters SpawnParams;
            const FVector SpawnLocation(0.0f, 0.0f, 50.0f);
            const FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

            AActor* SpawnedActor = World->SpawnActor<AActor>(Blueprint->GeneratedClass, SpawnLocation, SpawnRotation, SpawnParams);

            if (SpawnedActor) {
                SpawnedActor->SetFolderPath(FName(TEXT("Logi_ThermalCamera")));
                UE_LOG(LogTemp, Log, TEXT("Spawned blueprint in scene, located in Outliner-folder"));
            }
            else {
                UE_LOG(LogTemp, Error, TEXT("Could not spawn blueprint"));
            }
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Blueprint-cast failed."));
        }
        
    }

    
    void AddThermalPostProcessVolumeToOutliner(UWorld* World, bool& bSuccess, FString& StatusMessage)
    {
         const FString ThermalPostProcessVolumeLabel = TEXT("ThermalPostProcessVolume");

        // Check if ThermalPostProcessVolume already exists in Outliner/Scene
        AActor* ExistingActor = FindActorInOutlinerByLabel(World, ThermalPostProcessVolumeLabel);

        if (ExistingActor)
        {
            UE_LOG(LogTemp, Log, TEXT("ThermalPostProcessVolume already exists in scene under 'Logi_ThermalCamera'."));

            if (OverwritePopup(ThermalPostProcessVolumeLabel))
            {
                DeleteActorInOutliner(ExistingActor);
                CreateThermalPostProcessVolume(World, bSuccess, StatusMessage);
            }
            else
            {
                bSuccess = false;
                StatusMessage = TEXT("User chose not to overwrite existing ThermalPostProcessVolume.");
            }
        }
        else {

            // If not, create/add new
            CreateThermalPostProcessVolume(World, bSuccess, StatusMessage);
        }

    }

    
    void CreateThermalPostProcessVolume(UWorld* World, bool& bSuccess, FString& StatusMessage)
    {
        FActorSpawnParameters SpawnParams;
        const FVector SpawnLocation(0.0f, 0.0f, 0.0f);
        const FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

        APostProcessVolume* NewVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

        if (NewVolume)
        {
            NewVolume->SetActorLabel(TEXT("ThermalPostProcessVolume"));
            NewVolume->SetFolderPath(FName(TEXT("Logi_ThermalCamera")));

            NewVolume->bUnbound = true; // Infinite Extent setting

            UE_LOG(LogTemp, Log, TEXT("Spawned ThermalPostProcessVolume in Outliner."));
        }
        else
        {
            bSuccess = false;
            StatusMessage = TEXT("Failed to spawn ThermalPostProcessVolume.");
            return;
        }

        // Add PostProcess Material to the ThermalPostProcessVolume - PP_Logi_ThermalCamera 
        const FString MaterialPath = TEXT("/Game/Logi_ThermalCamera/Materials/PP_Logi_ThermalCamera");

        UMaterialInterface* PostProcessMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);

        if (PostProcessMaterial)
        {
            FWeightedBlendable Blendable;
            Blendable.Object = PostProcessMaterial;
            Blendable.Weight = 1.0f;

            NewVolume->Settings.WeightedBlendables.Array.Add(Blendable);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load PostProcessMaterial."));
        }

        const bool bSaved = LogiUtils::SaveAssetToDisk(NewVolume);

        if (bSaved)
        {
		    
            StatusMessage = FString::Printf(TEXT("Actor %s added and successfully saved to: %s"), *NewVolume->GetName(), *NewVolume->GetPathName());
            bSuccess = true;
        }
        else
        {
            StatusMessage = FString::Printf(TEXT("Actor %s added, but failed to save properly. Manual save will be required: %s"), *NewVolume->GetName(), *NewVolume->GetPathName());
            bSuccess = true;
        }

    }
}
