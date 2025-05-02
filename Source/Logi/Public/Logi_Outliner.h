
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"

class AActor;

class FLogiOutliner
{
public:
    static void AddLogiLogicToOutliner(UWorld* World, bool& success, FString& statusMessage);

private:
    static void AddThermalCameraToOutliner(UWorld* World);
    static void AddThermalPostProcessVolumeToOutliner(UWorld* World, bool& success, FString& statusMessage);

    static AActor* FindActorInOutlinerByLabel(UWorld* World, const FString& ActorLabel);

    static bool OverwritePopup(FString nameOfFile);
    static void DeleteActorInOutliner(AActor* Actor);

    static void SpawnNewBlueprint(UWorld* World, UBlueprint* Blueprint);
    static void CreateThermalPostProcessVolume(UWorld* World, bool& success, FString& statusMessage);
    
};
