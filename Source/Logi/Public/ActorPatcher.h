#pragma once
#include "K2Node_FunctionEntry.h"

namespace Logi::ActorPatcher
{
	void CreateThermalMaterial(bool& success, FString& statusMessage);

	static void FindAllNonLogiActorBlueprintsInProject(TArray<FAssetData>& OutActorBlueprints);

	static void AddLogiVariablesToActorBlueprint(const FAssetData& actor);

	static void AddNodeSetupToSetupFunction(UEdGraph* functionGraph, UK2Node_FunctionEntry* entryNode);

	static void AddNodeSetupToUpdateThermalMaterialFunction(UEdGraph* functionGraph, UK2Node_FunctionEntry* entryNode);

	static UEdGraph* AddSetupFunctionToNonLogiActor(const FAssetData& actor);
	
	static UEdGraph* AddUpdateThermalMaterialFunctionToNonLogiActor(const FAssetData& actor);

	static void MakeProjectBPActorsLogiCompatible();
};
