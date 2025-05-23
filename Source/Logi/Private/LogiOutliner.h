
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"

class AActor;

namespace Logi::LogiOutliner
{

    static void AddLogiLogicToOutliner(UWorld* World, bool& bSuccess, FString& StatusMessage);
    static void AddThermalCameraToOutliner(UWorld* World);
    static void AddThermalPostProcessVolumeToOutliner(UWorld* World, bool& bSuccess, FString& StatusMessage);

    static AActor* FindActorInOutlinerByLabel(const UWorld* World, const FString& ActorLabel);

    static bool OverwritePopup(const FString& NameOfFile);
    static void DeleteActorInOutliner(AActor* Actor);

    static void SpawnNewBlueprint(UWorld* World, const UBlueprint* Blueprint);
    static void CreateThermalPostProcessVolume(UWorld* World, bool& bSuccess, FString& StatusMessage);
    
};
