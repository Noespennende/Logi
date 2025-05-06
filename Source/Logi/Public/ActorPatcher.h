#pragma once
#include "K2Node_FunctionEntry.h"

namespace Logi::ActorPatcher
{
	void CreateThermalMaterial(bool& bSuccess, FString& StatusMessage);

	static void FindAllNonLogiActorBlueprintsInProject(TArray<FAssetData>& OutActorBlueprints);

	static void AddLogiVariablesToActorBlueprint(const FAssetData& Actor);

	static void AddNodeSetupToSetupFunction(UEdGraph* FunctionGraph, UK2Node_FunctionEntry* EntryNode);

	static void AddNodeSetupToUpdateThermalMaterialFunction(UEdGraph* FunctionGraph, UK2Node_FunctionEntry* EntryNode);

	static UEdGraph* AddSetupFunctionToNonLogiActor(const FAssetData& Actor);
	
	static UEdGraph* AddUpdateThermalMaterialFunctionToNonLogiActor(const FAssetData& Actor);

	static void MakeProjectBPActorsLogiCompatible();
};
