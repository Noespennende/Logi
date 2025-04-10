// Copyright Epic Games, Inc. All Rights Reserved.

#include "Logi.h"
#include "LogiStyle.h"
#include "LogiCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "../Public/LogiStyle.h"
#include "../Public/Logi.h"
#include "HAL/FileManager.h" // For editing files
#include "AssetViewUtils.h" 
#include "AssetRegistry/AssetRegistryModule.h"
#include "KismetCompilerModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"    
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "KismetCompiler.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_MathExpression.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "BlueprintNodeSpawner.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "K2Node_VariableSet.h"
#include "K2Node_MakeStruct.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include <functional>
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "K2Node_FunctionEntry.h"
#include "EdGraphSchema_K2_Actions.h"
#include "Engine/SimpleConstructionScript.h"
#include "MF_Logi_ThermalMaterialFunction.h"
#include "Engine/SCS_Node.h"
#include "Kismet/GameplayStatics.h"
#include "K2Node_GetArrayItem.h"
#include "MaterialDomain.h"
#include "Logi_Outliner.h"
#include "PP_Logi_ThermalCamera.h"


static const FName LogiTabName("Logi");

#define LOCTEXT_NAMESPACE "FLogiModule"

//Suport functions
void CreateFolder(FString folderPath, bool& folderCreated, FString& statusMessage) {

	// Convert project path to absolute path
	FString AbsoluteFolderPath;
	if (!FPackageName::TryConvertLongPackageNameToFilename(folderPath, AbsoluteFolderPath)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to convert folder path to absolute path: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//Check if folder already exists
	if (IFileManager::Get().DirectoryExists(*AbsoluteFolderPath)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Folder aldready exist: '% s'"), *AbsoluteFolderPath);
		return;
	}

	// Create a new folder
	if (!IFileManager::Get().MakeDirectory(*AbsoluteFolderPath, true)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to create folder: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//return success
	folderCreated = true;
	statusMessage = FString::Printf(TEXT("Creating folders succeeded - Folder created: '% s'"), *AbsoluteFolderPath);

}

void CreateFolderStructure(bool& folderCreated, FString& statusMessage) {

	// Create Logi_ThermalCamera folder
	CreateFolder("/Game/Logi_ThermalCamera", folderCreated, statusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	// Create Actor folder
	CreateFolder("/Game/Logi_ThermalCamera/Actors", folderCreated, statusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);


	//Create Material folder
	CreateFolder("/Game/Logi_ThermalCamera/Materials", folderCreated, statusMessage);
	// Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	//Create ActorMaterials folder inside the materials folder
	CreateFolder("/Game/Logi_ThermalCamera/Materials/ActorMaterials", folderCreated, statusMessage);
	// Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	// Set the folder color
	AssetViewUtils::SetPathColor("/Game/Logi_ThermalCamera", TOptional<FLinearColor>(FLinearColor(FColor::Cyan)));

}

UBlueprint* CreateBlueprintClass(FString filePath, TSubclassOf<UObject> ParentClass, bool& bOutSuccess, FString& statusMessage) {

	//Check if the file path is valid
	if (StaticLoadObject(UObject::StaticClass(), nullptr, *filePath) != nullptr) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - an asset already exists at that location  '%s'"), * filePath);
		return nullptr;
	}

	// Check if parent class is valid
	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass)) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - parent class is invalid '%s'"), *filePath);
		return nullptr;
	}

	// Create package
	UPackage* Package = CreatePackage(*filePath);
	if (Package == nullptr) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - failed to create package - Filepath is invalid '%s'"), *filePath);
		return nullptr;
	}

	// Find the blueprint class to create
	UClass* BpClass = nullptr;
	UClass* BpGenClass = nullptr;
	FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler").GetBlueprintTypesForClass(ParentClass, BpClass, BpGenClass);

	// Create blueprint
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, Package, *FPaths::GetBaseFilename(filePath), BPTYPE_Normal, BpClass, BpGenClass);

	// Register Blueprint in the asset registry
	FAssetRegistryModule::AssetCreated(Blueprint);
	Blueprint->MarkPackageDirty();

	// Return success
	bOutSuccess = true;
	statusMessage = FString::Printf(TEXT("Create blueprint success - created blueprint at '%s'"), *filePath);
	return Blueprint;

}

auto AddVariableToBlueprintClass(UBlueprint* blueprint, FName varName, FEdGraphPinType pinType, bool bInstanceEditable, FString defaultValue) {
	
	//Validate blueprint
	if (blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Adding variable to blueprint failed because the blueprint is null"));
		return;
	}

	// Check if the variable already exists, if so it skips the creation
	if (FBlueprintEditorUtils::FindNewVariableIndex(blueprint, varName) != INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in the blueprint '%s', skipping the creation of variable."), *varName.ToString(), *blueprint->GetName());
		return;
	}


	FBlueprintEditorUtils::AddMemberVariable(blueprint, varName, pinType, defaultValue);


	//Sets innstance editable
	if (bInstanceEditable) {
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, "MD_EditAnywhere", TEXT("true"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));

	}
}

TArray<FName> FindAllMeshComponentsInBlueprint(UBlueprint* blueprint) {
	TArray<FName> meshComponentNames;
	//Iterate over all nodes in the blueprint
	for (USCS_Node* node : blueprint->SimpleConstructionScript->GetAllNodes()) {
		//Check if the node is a nullptr if so, skip
		if (!node) continue;
		//Check if the node has a component class
		UClass* ComponentClass = node->ComponentClass;
		//Check if the component class is a child of the skeletal mesh component or static mesh component
		if (ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) || ComponentClass->IsChildOf(UStaticMeshComponent::StaticClass())) {
			//Add the variable name to the mesh component names list
			meshComponentNames.Add(node->GetVariableName());
		}
	}

	return meshComponentNames;
}

TArray<UMaterialInterface*> FindAllMaterialsFromActorScsNode(USCS_Node* scsNode) {
	TArray<UMaterialInterface*> materials;

	//Validate the scs node
	if (!scsNode) {
		UE_LOG(LogTemp, Error, TEXT("SCS node is null"));
		return materials;
	}

	//Get the mesh component from the SCS node
	UMeshComponent* meshComponent = Cast<UMeshComponent>(scsNode->ComponentTemplate);

	//Validate the mesh component
	if (!meshComponent) {
		UE_LOG(LogTemp, Error, TEXT("Mesh component is null, can't find materials"));
		return materials;
	}

	materials = meshComponent->GetMaterials();

	return materials;
}

TArray<USCS_Node*> FindUscsNodesForMeshComponentsFromABlueprint(UBlueprint* blueprint) {

	TArray<USCS_Node*> nodes;

	//validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
		return nodes;
	}

	for (USCS_Node* node : blueprint->SimpleConstructionScript->GetAllNodes()) {
		if (!node) continue;

		UClass* componentClass = node->ComponentClass;

		if (componentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) ||
			componentClass->IsChildOf(UStaticMeshComponent::StaticClass()))
		{
			nodes.Add(node);
		}
	}

	return nodes;

}

UMeshComponent* FindActorMeshComponentFromName(UBlueprint* blueprint, FName meshComponentName) {

	//Find the actor default object in the blueprint
	AActor* actorDefaultObject = Cast<AActor>(blueprint->GeneratedClass->GetDefaultObject());

	//Validate actor default object
	if (!actorDefaultObject) {
		UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
		return nullptr;
	}

	//Find the meshComponent in the blueprint by name
	for (UActorComponent* component : actorDefaultObject->GetComponents())
	{
		if (component && component->GetFName() == meshComponentName)
		{
			return Cast<UMeshComponent>(component);
		}
	}

	return nullptr;
}

UMaterialInterface* CreateActorLogiMaterial(UMaterialInterface* material, FName actorName) {

	//Validate material
	if (!material) {
		UE_LOG(LogTemp, Error, TEXT("Material is null, can't create Logi material"));
		return nullptr;
	}

	//Load Logi_ThermalMaterialFunction
	UMaterialFunctionInterface* thermalMaterialFunction = LoadObject<UMaterialFunctionInterface>(nullptr, TEXT("/Game/Logi_ThermalCamera/Materials/MF_Logi_ThermalMaterialFunction.MF_Logi_ThermalMaterialFunction"));

	//Validate thermal material function
	if (!thermalMaterialFunction) {
		UE_LOG(LogTemp, Error, TEXT("Logi_ThermalMaterialFunction is null, can't create Logi material"));
		return nullptr;
	}

	//Load Logi_ThermalSettings
	UMaterialParameterCollection* thermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings.MPC_Logi_ThermalSettings"));

	//validate thermal settings
	if (!thermalSettings) {
		UE_LOG(LogTemp, Error, TEXT("Logi_ThermalSettings is null, can't create Logi material"));
		return nullptr;
	}

	//Load blending function
	UMaterialFunctionInterface* blendingFunction = LoadObject<UMaterialFunctionInterface>(nullptr, TEXT("/Engine/Functions/Engine_MaterialFunctions01/LayerBlend/MatLayerBlend_Simple.MatLayerBlend_Simple"));

	//Validate blending function
	if (!blendingFunction) {
		UE_LOG(LogTemp, Error, TEXT("Blending function is null, can't create Logi material"));
		return nullptr;
	}

	//generate the material name
	FString originalMaterialName = material->GetName();
	FString LogiMaterialName = FString::Printf(TEXT("M_Logi_%s_%s"), *actorName.ToString(), *originalMaterialName);

	//Generating save path for new material
	FString savePath = FString::Printf(TEXT("/Game/Logi_ThermalCamera/Materials/ActorMaterials/%s"), *LogiMaterialName);
	UPackage* package = CreatePackage(*savePath);

	//Creating new material
	UMaterial* logiMaterial = NewObject<UMaterial>(package, *LogiMaterialName, RF_Public | RF_Standalone);

	//Set material properties
	logiMaterial->MaterialDomain = MD_Surface;
	logiMaterial->BlendMode = BLEND_Opaque;
	logiMaterial->SetShadingModel(MSM_DefaultLit);
	logiMaterial->bUseMaterialAttributes = true;

	return logiMaterial;

}

auto ReplaceComponentMaterialsWithLogiMaterialsInMeshComponent(UBlueprint* blueprint) {
	//validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't replace materials"));
		return;
	}

	TArray<USCS_Node*> meshComponentUscsNodes = FindUscsNodesForMeshComponentsFromABlueprint(blueprint);

	//Validate meshComponentUscsNodes
	if (meshComponentUscsNodes.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("No mesh component nodes found in the blueprint"));
		return;
	}


	//Replace materials in all actor mesh components with Logi materials
	for (USCS_Node* node : meshComponentUscsNodes) {
		TArray<UMaterialInterface*> materials = FindAllMaterialsFromActorScsNode(node);

		//Validate materials
		if (materials.Num() == 0) {
			UE_LOG(LogTemp, Error, TEXT("No materials found in the mesh component"));
			continue;
		}

		//Replace materials in the mesh component with Logi materials
		for (UMaterialInterface* material : materials) {
			UMaterialInterface* logiMaterial = CreateActorLogiMaterial(material, blueprint->GetFName());

		}


	}


}

auto AddThermalControlerReferenceToBlueprint(UBlueprint* blueprint, FName varName, bool bInstanceEditable) {

	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't add reference"));
		return;
	}

	if (FBlueprintEditorUtils::FindNewVariableIndex(blueprint, varName) != INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in blueprint '%s'"), *varName.ToString(), *blueprint->GetName());
		return;
	}

	// Load the blueprint asset (not just the generated class directly)
	UBlueprint* controllerBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController.BP_Logi_ThermalController")));
	if (!controllerBP || !controllerBP->GeneratedClass) {
		UE_LOG(LogTemp, Error, TEXT("Could not load BP_Logi_ThermalController Blueprint or its GeneratedClass"));
		return;
	}

	// Use the generated class (this ensures Blueprint properties are recognized)
	UClass* controllerClass = controllerBP->GeneratedClass;

	// Create pin type
	FEdGraphPinType controllerRefType;
	controllerRefType.PinCategory = UEdGraphSchema_K2::PC_Object;
	controllerRefType.PinSubCategoryObject = controllerClass;

	// Add the variable
	FBlueprintEditorUtils::AddMemberVariable(blueprint, varName, controllerRefType);

	// Make it instance editable
	if (bInstanceEditable) {
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, TEXT("EditAnywhere"), TEXT("true"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
	}
}

UEdGraphNode* AddNodeToBlueprint(UBlueprint* Blueprint, FName FunctionName, UClass* Class, FVector Location)
{
	if (!Blueprint)
	{
		// Blueprint is null, handle the error
		return nullptr;
	}

	if (!Class)
	{
		// The blueprint class is null, handle the error
		return nullptr;
	}

	UFunction* Function = Class->FindFunctionByName(FunctionName);
	if (!Function)
	{
		// The function does not exist in the blueprint class, handle the error
		UE_LOG(LogTemp, Warning, TEXT("Function Not Found"));
		return nullptr;
	}

	// Get the Event Graph of the blueprint
	UEdGraph* EventGraph = Blueprint->UbergraphPages[0];

	// Create a new function node in the Event Graph
	UEdGraphNode* NewNode = NewObject<UEdGraphNode>(EventGraph);

	//create spawner to spawn the node
	UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);

	//spawn the node
	if (FunctionNodeSpawner)
	{
		FunctionNodeSpawner->SetFlags(RF_Transactional);
		NewNode = FunctionNodeSpawner->Invoke(EventGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(Location.X, Location.Y));
		return NewNode;
	}

	return nullptr;

}

UEdGraphNode* AddNodeToBlueprintFunction(UEdGraph* functionGraph, FName functionName, UClass* nodeClass, FVector location) {

	//validate function graph
	if (!functionGraph) {
		UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, can't add node"));
		return nullptr;
	}

	//Find node class function
	UFunction* function = nodeClass->FindFunctionByName(functionName);

	//Validate node class function
	if (!function)
	{
		// The function does not exist in the blueprint class, handle the error
		UE_LOG(LogTemp, Warning, TEXT("node class function not found"));
		return nullptr;
	}

	// Create a new function node in the function graph
	UEdGraphNode* newNode = NewObject<UEdGraphNode>(functionGraph);

	//Create spawner to spawn the node
	UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(function);

	//Spawn the node
	if (FunctionNodeSpawner)
	{
		FunctionNodeSpawner->SetFlags(RF_Transactional);
		newNode = FunctionNodeSpawner->Invoke(functionGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(location.X, location.Y));
		return newNode;
	}

	return nullptr;
}

UK2Node_IfThenElse* CreateBPBranchNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_IfThenElse* branchNode = NewObject<UK2Node_IfThenElse>(eventGraph);
	branchNode->AllocateDefaultPins();
	eventGraph->AddNode(branchNode);
	branchNode->NodePosX = xPosition;
	branchNode->NodePosY = yPosition;

	return branchNode;
}

UK2Node_VariableGet* CreateBPGetterNode(UEdGraph* eventGraph, FName variableName, int xPosition, int yPosition) {
	UK2Node_VariableGet* getNode = NewObject<UK2Node_VariableGet>(eventGraph);
	getNode->VariableReference.SetSelfMember(variableName);
	getNode->AllocateDefaultPins();
	eventGraph->AddNode(getNode);
	getNode->NodePosX = xPosition;
	getNode->NodePosY = yPosition;

	return getNode;
}

UK2Node_GetArrayItem* CreateBPArrayGetterNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
	UK2Node_GetArrayItem* arrayGetNode = NewObject<UK2Node_GetArrayItem>(functionGraph);
	functionGraph->AddNode(arrayGetNode);
	arrayGetNode->NodePosX = xPosition;
	arrayGetNode->NodePosY = yPosition;
	arrayGetNode->AllocateDefaultPins();

	return arrayGetNode;
}

UK2Node_VariableSet* CreateBPSetterNode(UEdGraph* functionGraph, FName variableName, int xPosition, int yPosition) {
	UK2Node_VariableSet* setterNode = NewObject<UK2Node_VariableSet>(functionGraph);
	functionGraph->AddNode(setterNode);
	setterNode->VariableReference.SetSelfMember(variableName);
	setterNode->NodePosX = xPosition;
	setterNode->NodePosY = yPosition;
	setterNode->AllocateDefaultPins();

	return setterNode;
}

UK2Node_CallFunction* CreateBPScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* scalarParameterNode = NewObject<UK2Node_CallFunction>(eventGraph);
	scalarParameterNode->FunctionReference.SetExternalMember(FName("SetScalarParameterValue"), UKismetMaterialLibrary::StaticClass());
	scalarParameterNode->AllocateDefaultPins();
	eventGraph->AddNode(scalarParameterNode);
	scalarParameterNode->NodePosX = xPosition;
	scalarParameterNode->NodePosY = yPosition;

	return scalarParameterNode;
}

UK2Node_CallFunction* CreateBPVectorParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* vectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
	vectorNode->FunctionReference.SetExternalMember(FName("SetVectorParameterValue"), UKismetMaterialLibrary::StaticClass());
	vectorNode->AllocateDefaultPins();
	eventGraph->AddNode(vectorNode);
	vectorNode->NodePosX = xPosition;
	vectorNode->NodePosY = yPosition;

	return vectorNode;
}

UK2Node_CallFunction* CreateBPNormalizeToRangeNode(UEdGraph* eventGraph, UK2Node_VariableGet* getNode, int xPosition, int yPosition) {
	UK2Node_CallFunction* normalizeToRangeNode = NewObject<UK2Node_CallFunction>(eventGraph);
	normalizeToRangeNode->FunctionReference.SetExternalMember(FName("NormalizeToRange"), UKismetMathLibrary::StaticClass());
	normalizeToRangeNode->AllocateDefaultPins();
	eventGraph->AddNode(normalizeToRangeNode);
	normalizeToRangeNode->NodePosX = xPosition;
	normalizeToRangeNode->NodePosY = yPosition;

	return normalizeToRangeNode;
}

UK2Node_CallFunction* CreateBPMakeVectorNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* makeVectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
	makeVectorNode->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, MakeVector),
		UKismetMathLibrary::StaticClass()
	);
	makeVectorNode->AllocateDefaultPins();
	eventGraph->AddNode(makeVectorNode);
	makeVectorNode->NodePosX = xPosition;
	makeVectorNode->NodePosY = yPosition;

	return makeVectorNode;
}

UK2Node_CallFunction* CreateBPSetRenderDepthNode(UEdGraph* functionGraph, bool defaultValue, int xPosition, int yPosition) {
	UK2Node_CallFunction* setRenderDepthNode = NewObject<UK2Node_CallFunction>(functionGraph);
	setRenderDepthNode->FunctionReference.SetExternalMember(FName("SetRenderCustomDepth"), UPrimitiveComponent::StaticClass());
	setRenderDepthNode->AllocateDefaultPins();
	functionGraph->AddNode(setRenderDepthNode);
	setRenderDepthNode->NodePosX = xPosition;
	setRenderDepthNode->NodePosY = yPosition;

	//Finds the value pin
	UEdGraphPin* valuePin = setRenderDepthNode->FindPin(FName("bValue"));

	//Sets the value pin as default value
	if (valuePin) {
		valuePin->DefaultValue = defaultValue ? TEXT("true") : TEXT("false");
	}

	return setRenderDepthNode;
}

UK2Node_CallFunction* CreateBPGetAllActorsOfClassNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* getAllActorsNode = NewObject<UK2Node_CallFunction>(functionGraph);
	getAllActorsNode->FunctionReference.SetExternalMember(FName("GetAllActorsOfClass"), UGameplayStatics::StaticClass());
	getAllActorsNode->NodePosX = xPosition;
	getAllActorsNode->NodePosY = yPosition;
	getAllActorsNode->AllocateDefaultPins();
	functionGraph->AddNode(getAllActorsNode);

	return getAllActorsNode;
}

void CreateThermalCameraControllerNodeSetup(UBlueprint* blueprint) {

	//Validate blueprint
	if (blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint is null"));
		return;
	}

	//Finds the event graph for the blueprint
	UEdGraph* eventGraph = nullptr;
	for (UEdGraph* Graph : blueprint->UbergraphPages) {
		if (Graph->GetFName() == FName(TEXT("EventGraph"))) {
			eventGraph = Graph;
			break;
		}
	}

	//Validates the blueprint event graph
	if (eventGraph == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint graph is null"));
		return;
	}

	//Get the event graph schema
	const UEdGraphSchema_K2* Schema = CastChecked<UEdGraphSchema_K2>(eventGraph->GetSchema());

	//Find the event tick node in the event graph
	UK2Node_Event* EventTick = nullptr;
	//Iterate over all the nodes in the blueprints event graph
	for (UEdGraphNode* Node : eventGraph->Nodes) {
		//Check if the node is an event node
		if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node)) {
			//Check if the event node is the event tick node
			if (EventNode->EventReference.GetMemberName() == FName("ReceiveTick")) {
				EventTick = EventNode;
				break;  
			}
		}
	}

	//Validates that we actually found the event tick node
	if (!EventTick) {
		UE_LOG(LogTemp, Error, TEXT("Failed to find the Event Tick node in blueprints event graph"));
		return;
	}


	//Create a new branch node
	UK2Node_IfThenElse* BranchNode = CreateBPBranchNode(eventGraph, 300, 420);

	//Connect the event tick node to the branch node
	UEdGraphPin* ExecPin = EventTick->FindPin(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* BranchExecPin = BranchNode->GetExecPin();
	if (ExecPin && BranchExecPin) {
		Schema->TryCreateConnection(ExecPin, BranchExecPin);
	}

	//Create ThermalCameraActive variable get node
	UK2Node_VariableGet* ThermalCameraActiveGetNode = CreateBPGetterNode(eventGraph, FName("ThermalCameraActive"), 100, 550);

	//Connect the ThermalCameraActive variable get node to the branch node
	Schema->TryCreateConnection(ThermalCameraActiveGetNode->FindPin(FName("ThermalCameraActive")), BranchNode->GetConditionPin());


	//Create scalar parameter node - Thermal Camera Toggle true
	UK2Node_CallFunction* thermalToggleTrueScalarParameterNode = CreateBPScalarParameterNode(eventGraph, 600, 200);
	//Set node input for parameterName and value for Thermal Camera Toggle true
	UEdGraphPin* ParamNamePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterName"));
	UEdGraphPin* ParamValuePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterValue"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "1.0";

	//Create scalar parameter node - Thermal Camera Toggle false
	UK2Node_CallFunction* thermalToggleFalseScalarParameterNode = CreateBPScalarParameterNode(eventGraph, 600, 600);
	//Set node input for parameterName and value for  ThermalCameraToggle False
	ParamNamePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterName"));
	ParamValuePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterValue"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "0.0";


	//Connect thermal camera toggle parameter node to branch nodes true execution pin
	Schema->TryCreateConnection(BranchNode->GetThenPin(), thermalToggleTrueScalarParameterNode->GetExecPin());

	//Connect thermal camera toggle parameter node to branch nodes false execution pin
	Schema->TryCreateConnection(BranchNode->GetElsePin(), thermalToggleFalseScalarParameterNode->GetExecPin());

	//Create vector parameter nodes for Cold, Mid, Hot, BackgroundTemp temperatures
	FVector2D nodePosition(1000, 420);
	TArray<FString> vectorParams = { "Cold", "Mid", "Hot"};
	UK2Node_CallFunction* previousNode = nullptr;

	//Create vector parameter nodes
	for (const FString& Param : vectorParams) {
		//Create a vector parameter node
		UK2Node_CallFunction* vectorNode = CreateBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

		// Find the parameter pin for the vector node and set the default value
		UEdGraphPin* ParamPin = vectorNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = CreateBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));
	

		// connect the parameter nodes
		if (Param == "Cold") {
			//Connect Cold scalar paremeter node to the branch node
			Schema->TryCreateConnection(thermalToggleTrueScalarParameterNode->GetThenPin(), vectorNode->GetExecPin());
			Schema->TryCreateConnection(thermalToggleFalseScalarParameterNode->GetThenPin(), vectorNode->GetExecPin());
		}
		else {
			Schema->TryCreateConnection(vectorNode->GetExecPin(), previousNode->GetThenPin());
		}


		//Connect connect get node to the vector parameter node
		Schema->TryCreateConnection(getNode->FindPin(FName(*Param)), vectorNode->FindPin(FName("ParameterValue")));
		
		
		//Move node to the right for each iteration
		nodePosition.X += 400;

		//Set previous node
		previousNode = vectorNode;
	}


	//Create Scalar parameter nodes
	TArray<FString> scalarParams = { "BackgroundTemperature","SkyTemperature", "Blur", "NoiseAmount"};
	for (const FString& Param : scalarParams) {

		//Create a scalar parameter node
		UK2Node_CallFunction* scalarNode = CreateBPScalarParameterNode(eventGraph, nodePosition.X, nodePosition.Y);


		// Find the parameterPin for the scalar node and set the default value
		UEdGraphPin* ParamPin = scalarNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = CreateBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));

		//Spawn normalize to range node
		UK2Node_CallFunction* normalizeToRangeNode = CreateBPNormalizeToRangeNode(eventGraph, getNode, (nodePosition.X), (nodePosition.Y + 350));
		//Find normalize to range nodes range min and max pins
		UEdGraphPin* rangeMinPin = normalizeToRangeNode->FindPin(FName("RangeMin"));
		UEdGraphPin* rangeMaxPin = normalizeToRangeNode->FindPin(FName("RangeMax"));

		//Spawn getter node for camera range and add them to the background temparature's normalize node
		if (Param == "BackgroundTemperature" || Param == "SkyTemperature") {
			//Spawn thermal camera range getters
			UK2Node_VariableGet* thermalCameraRangeMinNode = CreateBPGetterNode(eventGraph, FName("ThermalCameraRangeMin"), getNode->NodePosX-150, (nodePosition.Y + 400));
			UK2Node_VariableGet* thermalCameraRangeMaxNode = CreateBPGetterNode(eventGraph, FName("ThermalCameraRangeMax"), getNode->NodePosX-150, (nodePosition.Y + 500));


			//Connect getter nodes to normalize to range node
			Schema->TryCreateConnection(thermalCameraRangeMinNode->FindPin(FName("ThermalCameraRangeMin")), normalizeToRangeNode->FindPin(FName("RangeMin")));
			Schema->TryCreateConnection(thermalCameraRangeMaxNode->FindPin(FName("ThermalCameraRangeMax")), normalizeToRangeNode->FindPin(FName("RangeMax")));
		}
		else {
			//Set normalize range's min and max values
			rangeMinPin->DefaultValue = TEXT("0.0");
			rangeMaxPin->DefaultValue = TEXT("100.0");
		}

		//Connect the getter to the normalize node
		Schema->TryCreateConnection(getNode->FindPin(FName(*Param)), normalizeToRangeNode->FindPin(FName("Value")));

		//Connect the normalize node to the scalar node
		Schema->TryCreateConnection(normalizeToRangeNode->FindPin(FName("ReturnValue")), scalarNode->FindPin(FName("ParameterValue")));

		//Connect the scalar node to the previous node
		Schema->TryCreateConnection(scalarNode->GetExecPin(), previousNode->GetThenPin());
		
		//Move node to the right for each iteration
		nodePosition.X += 400;

		//Set previous node
		previousNode = scalarNode;


	}

	//create NoiseSize variable getter node
	UK2Node_VariableGet* noiseSizeGet = CreateBPGetterNode(eventGraph, FName("NoiseSize"), nodePosition.X, (nodePosition.Y + 200));

	//Update node position
	nodePosition.X += 250;

	//Create makevector node to convert NoiseSize in to a 3D vector
	UK2Node_CallFunction* makeVectorNode = CreateBPMakeVectorNode(eventGraph, nodePosition.X, (nodePosition.Y + 200));

	//Create NoiseVector setter node
	UK2Node_VariableSet* setVectorNode = CreateBPSetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, nodePosition.Y);

	//Connect NoiseSize to MakeVector.X/Y/Z
	UEdGraphPin* NoiseOut = noiseSizeGet->GetValuePin();
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("X")));
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("Y")));
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("Z")));

	//Connect makeVector to setVector node
	Schema->TryCreateConnection(makeVectorNode->GetReturnValuePin(), setVectorNode->FindPin(FName("NoiseVector")));
	
	//Connect setVector node to previous node
	Schema->TryCreateConnection(setVectorNode->GetExecPin(), previousNode->GetThenPin());

	//Update node position
	nodePosition.X += 250;

	//Create Noisesize setVectorParameterValue node
	UK2Node_CallFunction* noiseSizeVectorNode = CreateBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

	// Set setVectorParameterValue node parameter name to NoiseSize
	UEdGraphPin* noiseSizevectorNodeParamPin = noiseSizeVectorNode->FindPin(FName("ParameterName"));
	noiseSizevectorNodeParamPin->DefaultValue = FString("NoiseSize");

	//Connect NoiseSize setVectorParameterValue node to setVector node
	Schema->TryCreateConnection(noiseSizeVectorNode->GetExecPin(), setVectorNode->GetThenPin());


	//Create getter node for the NoiseVector variable
	UK2Node_VariableGet* getNoiseVectorNode = CreateBPGetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, (nodePosition.Y + 300));

	//Connect the NoiseVector getter node to the NoiseSize setVectorParameterValue node
	Schema->TryCreateConnection(getNoiseVectorNode->FindPin(FName("NoiseVector")), noiseSizeVectorNode->FindPin(FName("ParameterValue")));


	//Compile the blueprint
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

}

void CreateThermalController(bool& success, FString& statusMessage) {
	//Create thermal controller blueprint
	UBlueprint* thermalControllerBp = CreateBlueprintClass("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController", AActor::StaticClass(), success, statusMessage);

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	//Check if blueprint is valid
	if (!thermalControllerBp) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create Thermal Controller Blueprint - Blueprint is NULL!"));
		success = false;
		statusMessage = "Failed to create Thermal Controller Blueprint - Blueprint is NULL!";
		return;
	}

	//Create bolean type
	FEdGraphPinType BoolType;
	BoolType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

	//Create float type
	FEdGraphPinType FloatType;
	FloatType.PinCategory = UEdGraphSchema_K2::PC_Real;
	FloatType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	//Create Linear Color type
	FEdGraphPinType LinearColorType;
	LinearColorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	LinearColorType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();

	//Create vector type
	FEdGraphPinType VectorType;
	VectorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	VectorType.PinSubCategoryObject = TBaseStructure<FVector>::Get();

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Adding variables to thermal controller blueprint"));

	//Add variables to the blueprint
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraActive", BoolType, true, "false");
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMin", FloatType, true, "0.0");
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMax", FloatType, true, "25.0");
	AddVariableToBlueprintClass(thermalControllerBp, "BackgroundTemperature", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "SkyTemperature", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Blur", FloatType, true, "100");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseSize", FloatType, true, "1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseAmount", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Cold", LinearColorType, true, "R=0.0,G=0.0,B=0.0,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Mid", LinearColorType, true, "R=0.5,G=0.5,B=0.5,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Hot", LinearColorType, true, "R=1.0,G=1.0,B=1.0,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseVector", VectorType, false, "0, 0, 0");


	//Add nodes to the blueprint
	CreateThermalCameraControllerNodeSetup(thermalControllerBp);
	
	//Validates blueprint
	if (!thermalControllerBp->GeneratedClass) {
		success = false;
		statusMessage = "Thermal Controller Blueprint has an invalid generated class.";
		return;
	}

	//Gets the actors default object
	AActor* ActorDefaultObject = Cast<AActor>(thermalControllerBp->GeneratedClass->GetDefaultObject());

	//Sets default values to variables
	if (ActorDefaultObject) {

		//Creates anonymus functions to set default values to variables for each variabel type
		auto SetFloatProperty = [&](FName PropertyName, float Value) {
			FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (FloatProp) {
				FloatProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetBoolProperty = [&](FName PropertyName, bool Value) {
			FBoolProperty* BoolProp = FindFProperty<FBoolProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (BoolProp) {
				BoolProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetLinearColorProperty = [&](FName PropertyName, FLinearColor Value) {
			FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (StructProp && StructProp->Struct == TBaseStructure<FLinearColor>::Get()) {
				FLinearColor* ColorPtr = StructProp->ContainerPtrToValuePtr<FLinearColor>(ActorDefaultObject);
				*ColorPtr = Value;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetVectorProperty = [&](FName PropertyName, FVector Value) {
			FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (StructProp && StructProp->Struct == TBaseStructure<FVector>::Get()) {
				FVector* VectorPtr = StructProp->ContainerPtrToValuePtr<FVector>(ActorDefaultObject);
				*VectorPtr = Value;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		//Sets default values
		SetBoolProperty("ThermalCameraActive", true);
		SetFloatProperty("ThermalCameraRangeMin", 0.0f);
		SetFloatProperty("ThermalCameraRangeMax", 100.0f);
		SetFloatProperty("BackgroundTemperature", 25.0f);
		SetFloatProperty("Blur", 5.0f);
		SetFloatProperty("NoiseSize", 1.0f);
		SetFloatProperty("NoiseAmount", 0.5f);
		SetLinearColorProperty("Cold", FLinearColor(0.0f, 0.0f, 1.0f, 1.0f)); // Blue
		SetLinearColorProperty("Mid", FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
		SetLinearColorProperty("Hot", FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)); // Red
		SetVectorProperty("NoiseVector", FVector(0.0f, 0.0f, 0.0f));
	}

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Compiling thermal controller blueprint"));

	//Compile blueprint
	FKismetEditorUtilities::CompileBlueprint(thermalControllerBp);
	thermalControllerBp->MarkPackageDirty();

	success = true;
	statusMessage = FString::Printf(TEXT("Thermal controller blueprint created and compiled successfully"));
}

void FindAllNonLogiActorBlueprintsInProject(TArray<FAssetData>& OutActorBlueprints) {
	//Get the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	//Get the asset registry inside the asset registry module
	IAssetRegistry& Registry = AssetRegistryModule.Get();

	// verify registry is up to date
	Registry.SearchAllAssets(true);

	// Create filert for the search
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/Game"));
	Filter.bRecursivePaths = true;

	//create array to hold blueprints found
	TArray<FAssetData> AssetList;

	//Get all blueprints in the /games (content) folder and add them to the AssetsList list
	Registry.GetAssets(Filter, AssetList);

	//Filter out all blueprints that are not actors
	for (const FAssetData& Asset : AssetList) {

		// Skipping the Logi_ThermalCamera folder
		if (Asset.PackagePath.ToString().StartsWith("/Game/Logi_ThermalCamera")) {
			continue;
		}

		//Check if the blueprint has a parent class
		FString ParentClassPath;

		//Get the parent class path and set it to the ParentClassPath variable to that path
		if (Asset.GetTagValue<FString>("ParentClass", ParentClassPath))
		{
			// Remove the autogenerated "_C" suffix from the class path
			ParentClassPath.RemoveFromEnd(TEXT("_C"), ESearchCase::IgnoreCase);

			//Tries to find the actualparent class object from the classe's path
			UClass* ParentClass = FindObject<UClass>(nullptr, *ParentClassPath);

			//Check if the parent class is a child of the AActor, i.e if it's an Actor blueprint
			if (ParentClass && ParentClass->IsChildOf(AActor::StaticClass()))
			{
				//Actor blueprint added to the list
				OutActorBlueprints.Add(Asset);
			}
		}
	}
}

void AddLogiVariablesToActorBlueprint(const FAssetData& actor) {

	//Cast asset data to blueprint type
	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());

	//Validate that the cast was successfull
	if(!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load blueprint from asset data: %s"), *actor.AssetName.ToString());
		return;
	}

	//Create bolean type
	FEdGraphPinType boolType;
	boolType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

	//Create float type
	FEdGraphPinType floatType;
	floatType.PinCategory = UEdGraphSchema_K2::PC_Real;
	floatType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	//Create Linear Color type
	FEdGraphPinType linearColorType;
	linearColorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	linearColorType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();

	//Create vector type
	FEdGraphPinType vectorType;
	vectorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	vectorType.PinSubCategoryObject = TBaseStructure<FVector>::Get();

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Adding variable to actor blueprint"));

	//Add variables to the blueprint
	AddVariableToBlueprintClass(blueprint, "Logi_Hot", boolType, true, "false");
	AddVariableToBlueprintClass(blueprint, "Logi_BaseTemperature", floatType, true, "0.0");
	AddVariableToBlueprintClass(blueprint, "Logi_MaxTemperature", floatType, true, "25.0");
	AddVariableToBlueprintClass(blueprint, "Logi_CurrentTemperature", floatType, true, "25.0");

}

auto ReplaceActorMaterialsWithLogiMaterials(const FAssetData& actor) {
	//Cast asset data to blueprint type
	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());

	//Validate that the cast was successfull
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load blueprint from asset data: %s"), *actor.AssetName.ToString());
		return;
	}

	//Check every variable in the blueprint for mesh components
	TArray<FName> meshComponentNames = FindAllMeshComponentsInBlueprint(blueprint);

	//Check if the meshVariableName is empty if so, skipping implementation.
	if (meshComponentNames.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh component found in blueprint '%s', skipping implementation."), *blueprint->GetName());
		return;
	}

	for (FName meshComponentName : meshComponentNames) {
		ReplaceComponentMaterialsWithLogiMaterialsInMeshComponent(blueprint);
	}



}

auto AddNodeSetupToSetupFunction(UEdGraph* functionGraph, UK2Node_FunctionEntry* entryNode) {
	//Validate function graph

	if (!functionGraph) {
		UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, cannot add nodes to the graph."));
		return nullptr;
	}

	//validate entry node
	if (!entryNode) {
		UE_LOG(LogTemp, Error, TEXT("No function entry node is a nullpointer. Cannot add nodes to the graph."));
		return nullptr;
	}

	// Find the function blueprint
	UBlueprint* blueprint = Cast<UBlueprint>(functionGraph->GetOuter());

	//Validate the blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Could not find the blueprint linked to the blueprint function"));
		return nullptr;
	}

	int xPosition = 300;

	//Check every variable in the blueprint for mesh components
	TArray<FName> meshComponentNames = FindAllMeshComponentsInBlueprint(blueprint);

	//Check if the meshVariableName is empty if so, skipping implementation.
	if (meshComponentNames.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No skeletal mesh component found in blueprint '%s', skipping implementation."), *blueprint->GetName());
		return nullptr;
	}

	//Get the function graphs schema
	const UEdGraphSchema_K2* Schema = CastChecked<UEdGraphSchema_K2>(functionGraph->GetSchema());

	//Create Logi_Hot variable getter node
	UK2Node_VariableGet* logiHotGetNode = CreateBPGetterNode(functionGraph, FName("Logi_Hot"), xPosition, 200);

	//Update xPosition
	xPosition += 100;

	//Creating nodes inside the function graph
	UK2Node_IfThenElse* branchNode = CreateBPBranchNode(functionGraph, xPosition, 0);

	//Update xPosition
	xPosition += 300;

	//Connect the entry node to the branch node
	UEdGraphPin* execPin = entryNode->FindPin(UEdGraphSchema_K2::PN_Then);

	//Connect the Logi_Hot variable getter node to the branch node
	Schema->TryCreateConnection(logiHotGetNode->FindPin(FName("Logi_Hot")), branchNode->GetConditionPin());

	//Connect the entry node to the branch node
	Schema->TryCreateConnection(execPin, branchNode->GetExecPin());

	// Pointers to track the last SetRenderCustomDepth node in the true/false chain
	UK2Node_CallFunction* previousTrueExecNode = nullptr;
	UK2Node_CallFunction* previousFalseExecNode = nullptr;

	//Create mesh getter nodes and set render custom deapth node for each mesh in the blueprint
	for (int32 i = 0; i < meshComponentNames.Num(); ++i) {

		//Get the mesh variable name
		FName meshVariableName = meshComponentNames[i];

		//Create mesh getter nodes
		UK2Node_VariableGet* meshGetterNodeTrue = CreateBPGetterNode(functionGraph, meshVariableName, xPosition, -400);
		UK2Node_VariableGet* meshGetterNodeFalse = CreateBPGetterNode(functionGraph, meshVariableName, xPosition, 500);

		//Create set render custom depth nodes
		UK2Node_CallFunction* setRenderDepthNodeTrue = CreateBPSetRenderDepthNode(functionGraph, true, xPosition, -200);
		UK2Node_CallFunction* setRenderDepthNodeFalse = CreateBPSetRenderDepthNode(functionGraph, false, xPosition, 200);

		//Try to find the target pin for the set render custom depth nodes
		UEdGraphPin* trueTargetPin = setRenderDepthNodeTrue->FindPin(UEdGraphSchema_K2::PN_Self);
		UEdGraphPin* falseTargetPin = setRenderDepthNodeFalse->FindPin(UEdGraphSchema_K2::PN_Self);

		//Connect mesh getter nodes to set render custom depth nodes
		if (trueTargetPin && meshGetterNodeTrue->GetValuePin()) {
			Schema->TryCreateConnection(meshGetterNodeTrue->GetValuePin(), trueTargetPin);
		}

		if (falseTargetPin && meshGetterNodeFalse->GetValuePin()) {
			Schema->TryCreateConnection(meshGetterNodeFalse->GetValuePin(), falseTargetPin);
		}

		//Check if this is the first set render custom depth node in the chains
		if (i == 0) {
			//Connect the branch node to the set render custom depth nodes
			Schema->TryCreateConnection(branchNode->GetThenPin(), setRenderDepthNodeTrue->GetExecPin());
			Schema->TryCreateConnection(branchNode->GetElsePin(), setRenderDepthNodeFalse->GetExecPin());

			//Set the previous set render custom depth nodes to the current set render custom depth nodes
		}
		else {
			//Connect the previous set render custom depth node to the current set render custom depth node
			Schema->TryCreateConnection(previousTrueExecNode->GetThenPin(), setRenderDepthNodeTrue->GetExecPin());
			Schema->TryCreateConnection(previousFalseExecNode->GetThenPin(), setRenderDepthNodeFalse->GetExecPin());
		}

		//Set the previous set render custom depth nodes to the current set render custom depth nodes
		previousTrueExecNode = setRenderDepthNodeTrue;
		previousFalseExecNode = setRenderDepthNodeFalse;
	
		//Update xPosition
		xPosition += 300;
	}

	//create get all actors of class node
	UK2Node_CallFunction* getAllActorsOfClassNode = CreateBPGetAllActorsOfClassNode(functionGraph, xPosition, 0);

	//Connect Get all actors of class node to the render custom depth node.
	Schema->TryCreateConnection(previousTrueExecNode->GetThenPin(), getAllActorsOfClassNode->GetExecPin());
	Schema->TryCreateConnection(previousFalseExecNode->GetThenPin(), getAllActorsOfClassNode->GetExecPin());


	//find the actor class pin of the Get all actors of class node
	UEdGraphPin* actorClassPin = getAllActorsOfClassNode->FindPin(FName("ActorClass"));

	//validate the actor class pin
	if (!actorClassPin) {
		UE_LOG(LogTemp, Error, TEXT("Failed to find the actor class pin in the Get All Actors of Class node"));
		return nullptr;
	}

	//Find the Logi_ThermalController blueprint
	UBlueprint* controllerBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController.BP_Logi_ThermalController")));

	//Validate the thermal controller blueprint
	if (!controllerBP || !controllerBP->GeneratedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load BP_Logi_ThermalController or its generated class"));
		return nullptr;
	}

	//Set the actor class pin to the BP_Logi_ThermalController class
	actorClassPin->DefaultObject = controllerBP->GeneratedClass;

	//Notify actor class pin that the pin has changed
	actorClassPin->GetSchema()->TrySetDefaultObject(*actorClassPin, controllerBP->GeneratedClass);


	//update xPosition
	xPosition += 300;

	//Create array getter node
	UK2Node_GetArrayItem* arrayGetNode = CreateBPArrayGetterNode(functionGraph, xPosition, 200);

	//Find the array input pin of the array getter node
	UEdGraphPin* arrayInputPin = arrayGetNode->GetTargetArrayPin();


	//Connect the get all actors of class node to the array getter node
	Schema->TryCreateConnection(getAllActorsOfClassNode->FindPin(FName("OutActors")), arrayInputPin);

	//Notify the array getter node that the input pin has changed to update the pins name to the correct type
	arrayInputPin->GetOwningNode()->PinConnectionListChanged(arrayInputPin);

	//Reconstruct pins for the array getter node to account for the array type
	arrayGetNode->ReconstructNode();


	//Find the index input pin of the array getter node
	UEdGraphPin* indexPin = nullptr;
	//Iterate trough all pins of the getter node
	for (UEdGraphPin* Pin : arrayGetNode->Pins)
	{
		//Check if the pin's direction is an input pin and if the type is an integer, if so it is the index pin.
		if (Pin && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Int && Pin->Direction == EGPD_Input)
		{
			indexPin = Pin;
			break;
		}
	}
	
	//Validate index pin and set the index pins value
	if (indexPin) {
		indexPin->DefaultValue = TEXT("0");
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to find the index pin in the array getter node"));
		return nullptr;
	}

	//Update xPosition
	xPosition += 300;

	//Set thermal controller variable name
	FName thermalControllerVariableName = FName("Logi_ThermalController");

	//Adds thermal controller referense variable
	AddThermalControlerReferenceToBlueprint(blueprint, thermalControllerVariableName, false);


	
	//Create a variable setter node for the Logi_ThermalController variable
	UK2Node_VariableSet* setThermalController = CreateBPSetterNode(functionGraph, thermalControllerVariableName, xPosition, 0);

	//Connect the array getter node to the setThermalController node
	Schema->TryCreateConnection(arrayGetNode->GetResultPin(), setThermalController->FindPin(thermalControllerVariableName));


	UEdGraphPin* thenPin = getAllActorsOfClassNode->FindPin(UEdGraphSchema_K2::PN_Then);
	if (!thenPin)
	{
		UE_LOG(LogTemp, Error, TEXT("GetAllActorsOfClass node does not have a Then pin!"));
	}

	//Connect the setter node for Logi thermal controller and get all actors of class node's exec pins
	Schema->TryCreateConnection(setThermalController->GetExecPin(), getAllActorsOfClassNode->GetThenPin());



	return nullptr;

}

UEdGraph* AddSetupFunctionToNonLogiActor(const FAssetData& actor) {

	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());
	FName functionName = FName("Logi_ThermalActorSetup");


	//Validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, cannot add setup function."));
		return nullptr;
	}

	// Check if the setup function already exists in the blueprint
	for (UEdGraph* Graph : blueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == functionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("Function '%s' already exists in blueprint '%s', skipping implementation."), *functionName.ToString(), *blueprint->GetName());
			return nullptr;
		}
	}


	// Create the setup function
	UEdGraph* newFunctionGraph = FBlueprintEditorUtils::CreateNewGraph(blueprint, functionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UFunction>(blueprint, newFunctionGraph,true, nullptr);

	// Find the setup function entry node
	UK2Node_FunctionEntry* entryNode = nullptr;
	for (UEdGraphNode* node : newFunctionGraph->Nodes)
	{
		entryNode = Cast<UK2Node_FunctionEntry>(node);
		if (entryNode)
			break;
	}

	//Mark blueprint as modified
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

	UE_LOG(LogTemp, Log, TEXT("Function 'Logi_ThermalActorSetup' successfully added to blueprint '%s'."), *blueprint->GetName());

	AddNodeSetupToSetupFunction(newFunctionGraph, entryNode);

	return newFunctionGraph;
}

UEdGraph* AddUpdateThermalMaterialFunctionToNonLogiActor(const FAssetData& actor) {

	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());
	FName functionName = FName("Logi_UpdateThermalMaterial");

	//Validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, cannot add setup function."));
		return nullptr;
	}

	// Check if the setup function already exists in the blueprint
	for (UEdGraph* Graph : blueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == functionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("Function '%s' already exists in blueprint '%s', skipping implementation."), *functionName.ToString(), *blueprint->GetName());
			return nullptr;
		}
	}


	// Create the setup function
	UEdGraph* newFunctionGraph = FBlueprintEditorUtils::CreateNewGraph(blueprint, functionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UFunction>(blueprint, newFunctionGraph, true, nullptr);

	// Find the setup function entry node
	UK2Node_FunctionEntry* entryNode = nullptr;
	for (UEdGraphNode* node : newFunctionGraph->Nodes)
	{
		entryNode = Cast<UK2Node_FunctionEntry>(node);
		if (entryNode)
			break;
	}


	//Mark blueprint as modified
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

	UE_LOG(LogTemp, Log, TEXT("Function 'Logi_ThermalActorSetup' successfully added to blueprint '%s'."), *blueprint->GetName());

	return newFunctionGraph;
}

void MakeProjectBPActorsLogiCompatible() {
	//Create a list to hold all the actor blueprints in the project
	TArray<FAssetData> projectActors;
	//Find all the blueprints of type Actor in the /games (content) folder and add them to the projectActors list
	FindAllNonLogiActorBlueprintsInProject(projectActors);

	//Add Logi variables to all the actor blueprints in the project
	for (FAssetData actor : projectActors) {
		//Prints status
		UE_LOG(LogTemp, Warning, TEXT("Adding Logi variables to actor: %s"), *actor.AssetName.ToString());

		//Add Logi variables to the actor blueprint
		AddLogiVariablesToActorBlueprint(actor);
		ReplaceActorMaterialsWithLogiMaterials(actor);
		AddSetupFunctionToNonLogiActor(actor);
		AddUpdateThermalMaterialFunctionToNonLogiActor(actor);
	}
}

//Plugin functions

void FLogiModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLogiStyle::Initialize();
	FLogiStyle::ReloadTextures();

	FLogiCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLogiCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLogiModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLogiModule::RegisterMenus));
}

void FLogiModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLogiStyle::Shutdown();

	FLogiCommands::Unregister();
}


void FLogiModule::PluginButtonClicked()
{
	bool success;
	FString statusMessage;

	// Create folder structure
	CreateFolderStructure(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World) {
		SetupThermalSettings(World);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to get UWorld!"));
	}

	//Create Thermal Controller blueprint
	CreateThermalController(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	

	// Create Thermal MaterialFunction
	FMF_ThermalMaterialFunction::CreateMaterialFunction(success, statusMessage);

	FPP_ThermalCamera::CreateThermalCamera(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);
	//Make all project actors logi compatible
	MakeProjectBPActorsLogiCompatible();

	FLogiOutliner::AddLogiLogicToOutliner(World);

}


void FLogiModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLogiCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLogiCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

UMaterialParameterCollection* FLogiModule::EnsureThermalSettingsExist(UWorld* World)
{
	if (!World) return nullptr;

	//Asset path
	FString AssetPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings");

	// Try to load the existing MPC
	UMaterialParameterCollection* ThermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, *AssetPath);

	//if the MPC does not exist, create new MPC
	if (!ThermalSettings)
	{
		UPackage* Package = CreatePackage(*FString("/Game/Logi_ThermalCamera/Materials"));
		ThermalSettings = NewObject<UMaterialParameterCollection>(Package, UMaterialParameterCollection::StaticClass(), FName("MPC_Logi_ThermalSettings"), RF_Public | RF_Standalone);
		if (ThermalSettings) {

			//Lambda functions for adding parameters to MPC
			auto AddScalar = [&ThermalSettings](const FName& Name, float DefaultValue) -> void {
				FCollectionScalarParameter Param;
				Param.ParameterName = Name;
				Param.DefaultValue = DefaultValue;
				ThermalSettings->ScalarParameters.Add(Param);
				};

			auto AddVector = [&ThermalSettings](const FName& Name, const FLinearColor& DefaultValue) -> void {
				FCollectionVectorParameter Param;
				Param.ParameterName = Name;
				Param.DefaultValue = DefaultValue;
				ThermalSettings->VectorParameters.Add(Param);
				};
				
			//Scalar parameters to be added
			AddScalar(FName("ThermalCameraToggle"), 0.0);
			AddScalar(FName("BackgroundTemperature"), 0.0);
			AddScalar(FName("Blur"), 0.0);
			AddScalar(FName("NoiseAmount"), 0.05);
			AddScalar(FName("SkyTemperature"), 0.0);


			//vector parameters to be added
			AddVector(FName("Cold"), FLinearColor(1, 1, 1, 1));
			AddVector(FName("Mid"), FLinearColor(0, 1, 0, 1));
			AddVector(FName("Hot"), FLinearColor(1, 0, 0, 1));
			AddVector(FName("NoiseSize"), FLinearColor(0, 0, 0, 0));

			//Save the MPC with parameters
			ThermalSettings->MarkPackageDirty();
			FAssetRegistryModule::AssetCreated(ThermalSettings);
			FString FilePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
			UPackage::SavePackage(Package, ThermalSettings, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
			UE_LOG(LogTemp, Warning, TEXT("Created MPC_Logi_ThermalSettings successfully."));
		}

		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create MPC_Logi_ThermalSettings."));
		}
	}

	return ThermalSettings;
}

void FLogiModule::SetupThermalSettings(UWorld* World)
{
	if (!World) return;

	// Checks if MPC exists
	UMaterialParameterCollection* ThermalSettings = EnsureThermalSettingsExist(World);
	if (!ThermalSettings) return;

	// Checks if MPC has required parameters
	bool bUpdated = false;

	// checks if mpc has required scalar parameters if not add them
	TArray<FName> ScalarsRequired = {
		FName("ThermalCameraToggle"),
		FName("BackgroundTemperature"),
		FName("Blur"),
		FName("NoiseAmount"),
		FName("SkyTemperature"),
	};

	for (const FName& ScalarName : ScalarsRequired)
	{
		bool bExists = ThermalSettings->ScalarParameters.ContainsByPredicate(
			[&ScalarName](const FCollectionScalarParameter& Param) { return Param.ParameterName == ScalarName; });

		if (!bExists)
		{
			FCollectionScalarParameter NewScalar;
			NewScalar.ParameterName = ScalarName;
			NewScalar.DefaultValue = (ScalarName == FName("NoiseAmount")) ? 0.05f : 0.0f;  
			ThermalSettings->ScalarParameters.Add(NewScalar);
			bUpdated = true;
		}
	}

	// checks if mpc has required vector parameters if not add them
	TArray<TPair<FName, FLinearColor>> VectorsRequired = {
		{ FName("Cold"), FLinearColor(1, 1, 1, 1) },
		{ FName("Mid"), FLinearColor(0, 1, 0, 1) },
		{ FName("Hot"), FLinearColor(1, 0, 0, 1) },
		{ FName("NoiseSize"), FLinearColor(0, 0, 0, 0) }
	};

	for (const auto& VectorPair : VectorsRequired)
	{
		bool bExists = ThermalSettings->VectorParameters.ContainsByPredicate(
			[&VectorPair](const FCollectionVectorParameter& Param) { return Param.ParameterName == VectorPair.Key; });

		if (!bExists)
		{
			FCollectionVectorParameter NewVector;
			NewVector.ParameterName = VectorPair.Key;
			NewVector.DefaultValue = VectorPair.Value;
			ThermalSettings->VectorParameters.Add(NewVector);
			bUpdated = true;
		}
	}

	// save the parameters that were added
	if (bUpdated)
	{
		ThermalSettings->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(ThermalSettings);
		FString AssetPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings");
		FString FilePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(ThermalSettings->GetOutermost(), ThermalSettings, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		UE_LOG(LogTemp, Warning, TEXT("Added missing parameters to MPC_Logi_ThermalSettings"));
		
	}

	// set values of MPC asset
	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(ThermalSettings);
	if (Instance)
	{
		Instance->SetScalarParameterValue(FName("ThermalCameraToggle"),0);
		Instance->SetScalarParameterValue(FName("BackgroundTemperature"), 0);
		Instance->SetScalarParameterValue(FName("Blur"), 0);
		Instance->SetScalarParameterValue(FName("NoiseAmount"), 0.05);
		Instance->SetScalarParameterValue(FName("SkyTemperature"), 0);

		Instance->SetVectorParameterValue(FName("Cold"), FLinearColor(0,0,0,0));
		Instance->SetVectorParameterValue(FName("Mid"), FLinearColor(0.5, 0.5, 0.5, 0));
		Instance->SetVectorParameterValue(FName("Hot"), FLinearColor(1,1,1,0));
		Instance->SetVectorParameterValue(FName("NoiseSize"), FLinearColor(1,1,1,0));

		UE_LOG(LogTemp, Warning, TEXT("Thermal settings applied."));
	}

	
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogiModule, Logi)