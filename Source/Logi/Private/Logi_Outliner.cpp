#include "Logi_Outliner.h"
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
#include "Widgets/Text/STextBlock.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"



// Main method, that adds everything to the Outliner 
void FLogiOutliner::AddLogiLogicToOutliner(UWorld* World) {

    AddThermalCameraToOutliner(World);
    AddThermalPostProcessVolumeToOutliner(World);

}


void FLogiOutliner::AddThermalCameraToOutliner(UWorld* World) {
    
    
    FString BlueprintPath = "/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController";

    FString BlueprintLabel = TEXT("BP_Logi_ThermalController");

    UObject* BlueprintObject = StaticLoadObject(UObject::StaticClass(), nullptr, *BlueprintPath);
    
    if (!BlueprintObject) {
        UE_LOG(LogTemp, Error, TEXT("Could not load blueprint: %s"), *BlueprintPath);
        return;  
    }

    UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintObject);

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
        // If blueprint does not exists, spawn/add the blueprint
        SpawnNewBlueprint(World, Blueprint);
    }

}


// Show overwrite pop-up
bool FLogiOutliner::OverwritePopup(FString nameOfFile) {
    // Pop-up Yes/No alternatives
    
    FText DialogText = FText::Format(FText::FromString(TEXT("The file '{0}' already exists in the scene. Do you want to overwrite it?")), FText::FromString(nameOfFile));
    EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogText);

    return Response == EAppReturnType::Yes; // If user chooses "Yes", return true
}

// Find Actor with the given label in the Outliners Logi_ThermalCamera folder. Returns the actor if existing, returns nullptr if not.
AActor* FLogiOutliner::FindActorInOutlinerByLabel(UWorld* World, const FString& ActorLabel)
{
    const FString TargetFolder = TEXT("Logi_ThermalCamera");

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;

        if (Actor->GetFolderPath().ToString().Equals(TargetFolder) &&
            Actor->GetActorLabel().Equals(ActorLabel))
        {
            UE_LOG(LogTemp, Log, TEXT("Found existing actor: %s in folder %s"), *ActorLabel, *TargetFolder);
            return Actor;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Actor %s does not exist in folder %s"), *ActorLabel, *TargetFolder);
    return nullptr;
}


// Delete existing actor from Outliner (NOT any file content browser - but in Outliner/Scene)
void FLogiOutliner::DeleteActorInOutliner(AActor* Actor) {
    
    if (Actor)
    {
        UE_LOG(LogTemp, Log, TEXT("Deleting actor: %s"), *Actor->GetActorLabel());
        Actor->Destroy();
    }
}


// Spawn/add given blueprint to Outliner(Scene)
void FLogiOutliner::SpawnNewBlueprint(UWorld* World, UBlueprint* Blueprint) {
    
        if (Blueprint && Blueprint->GeneratedClass) {
            FActorSpawnParameters SpawnParams;
            FVector SpawnLocation(0.0f, 0.0f, 50.0f);
            FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

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



void FLogiOutliner::AddThermalPostProcessVolumeToOutliner(UWorld* World)
{
    
    
    FString ThermalPostProcessVolumeLabel = TEXT("ThermalPostProcessVolume");

    // Check if ThermalPostProcessVolume already exists in Outliner/Scene
    AActor* ExistingActor = FindActorInOutlinerByLabel(World, ThermalPostProcessVolumeLabel);

    if (ExistingActor) {

        UE_LOG(LogTemp, Log, TEXT("ThermalPostProcessVolume already exists in scene under 'Logi_ThermalCamera'."));

        if (OverwritePopup(ThermalPostProcessVolumeLabel)) {


            DeleteActorInOutliner(ExistingActor);
            CreateThermalPostProcessVolume(World);
        }

    }
    else {

        // If not, create/add new
        CreateThermalPostProcessVolume(World);
    }

}



void FLogiOutliner::CreateThermalPostProcessVolume(UWorld* World) {
    
    FActorSpawnParameters SpawnParams;
    FVector SpawnLocation(0.0f, 0.0f, 0.0f);
    FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

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
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn ThermalPostProcessVolume."));
    }

    // Add PostProcess Material to the ThermalPostProcessVolume - PP_ThermalCamera 
    // !!! Switch out "/Game/IR_Materials/PP_ThermalCamera" with "/Game/Logi_ThermalCamera/Materials/PP_ThermalCamera" when added
    UMaterialInterface* PostProcessMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Logi_ThermalCamera/Materials/PP_Logi_ThermalCamera")); 
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

}